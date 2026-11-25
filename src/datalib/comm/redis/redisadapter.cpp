/**
 * @file redisadapter.cpp
 * @brief Redis 协议适配器实现
 * @details 用 hiredis 直接订阅 AFSIM 发布频道，完整处理链路：
 *          RESP message → gzip 解压 → protobuf Envelope → Customize_Info
 *          → PlatformData → 主线程 DataManager / 态势显示转发。
 *          CPSS_ENABLE_REDIS 未定义时为桩实现（Debug 配置）。
 * @date 2026-11-22
 */

#include "redisadapter.h"
#include "logger.h"

#ifndef CPSS_ENABLE_REDIS
// ---- 桩实现（Debug 或 SDK 未就绪）----
static inline void redisLog(const QString &msg) {
    Logger::info("[RedisAdapter] %s", msg.toUtf8().constData());
}
RedisAdapter::RedisAdapter(QObject *parent) : IProtocolAdapter(parent) {}
RedisAdapter::~RedisAdapter() {}
bool RedisAdapter::start() {
    m_lastError = QStringLiteral("Redis 未启用：请用 Release 配置构建（Redis SDK 仅提供 Release 版，Debug 会因 CRT 冲突崩溃）");
    redisLog(m_lastError);
    m_status = Error;
    emit errorOccurred(m_lastError);
    return false;
}
bool RedisAdapter::stop() {
    m_status = Stopped;
    return true;
}
void RedisAdapter::onPlatformReceived(const PlatformData &platform) { Q_UNUSED(platform); }
#else

// ---- 真实实现（Release + Redis SDK）----
#include "redis_fieldmap.h"
#include "ddsconverter.h"       // 复用 JSON 中间格式转换
#include "situationforwarder.h" // 态势显示转发（RTI DDS）
#include "redisConnection.h"
#include "hiredis/hiredis.h"
#include "zlib.h"

// Windows 下 hiredis 的 timeval（不 include sockcompat.h，避免其 connect 宏污染 Qt 信号槽）
#ifdef _WIN32
#ifndef CPSS_TIMEVAL_DEFINED
#define CPSS_TIMEVAL_DEFINED
struct timeval { long tv_sec; long tv_usec; };
#endif
#endif
#include "redis_protocl.h"      // 必须先定义 HTPROTO_API 导出宏
#include "protocol.pb.h"        // HTProto::Envelope / Customize_Info
#include "datamanager.h"        // 强类型入口 updatePlatformDirect

#include <QtGlobal>
#include <QByteArray>
#include <cstring>

static inline void redisLog(const QString &msg) {
    Logger::info("[RedisAdapter] %s", msg.toUtf8().constData());
}

/** @brief gzip 解压（AFSIM 发布的 payload 是 gzip 压缩的 protobuf） */
static bool gunzipBuffer(const char *in, size_t inLen, std::string &out)
{
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    // 15+16 = 自动识别 gzip 头
    if (inflateInit2(&strm, 15 + 16) != Z_OK) return false;

    const size_t CHUNK = 4096;
    char buf[CHUNK];
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(in));
    strm.avail_in = static_cast<uInt>(inLen);

    bool ok = true;
    int ret;
    do {
        strm.next_out = reinterpret_cast<Bytef*>(buf);
        strm.avail_out = CHUNK;
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) { ok = false; break; }
        size_t got = CHUNK - strm.avail_out;
        if (got) out.append(buf, got);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return ok;
}

RedisAdapter::RedisAdapter(QObject *parent)
    : IProtocolAdapter(parent)
{
    // 关键：QueuedConnection 跨线程传自定义类型必须先注册元类型，否则信号被丢弃
    qRegisterMetaType<PlatformData>("PlatformData");

    // SDK 回调在订阅线程，用 QueuedConnection 切回主线程
    connect(this, &RedisAdapter::platformReceived,
            this, &RedisAdapter::onPlatformReceived, Qt::QueuedConnection);
}

RedisAdapter::~RedisAdapter()
{
    stop();
}

bool RedisAdapter::start()
{
    if (m_status == Running) return true;

    // 用一个 RedisConnection 实例持有（保证 DLL 侧的全局初始化），但订阅用独立 context
    try {
        m_connection = std::make_shared<RedisConnection>(
            m_host.toStdString().c_str(), m_port, nullptr, 0);
    } catch (const std::exception &e) {
        m_lastError = QString::fromUtf8(e.what());
        redisLog(QStringLiteral("start error: ") + m_lastError);
        m_status = Error;
        emit errorOccurred(m_lastError);
        return false;
    }

    // 建立专用订阅连接（带超时，便于 stop 时退出线程）
    struct timeval tv = { 1, 0 };  // 1 秒超时
    m_subContext = redisConnectWithTimeout(m_host.toStdString().c_str(), m_port, tv);
    if (!m_subContext || m_subContext->err) {
        m_lastError = m_subContext ? QString::fromUtf8(m_subContext->errstr)
                                   : QStringLiteral("redisConnectWithTimeout failed");
        redisLog(QStringLiteral("connect error: ") + m_lastError);
        if (m_subContext) { redisFree(m_subContext); m_subContext = nullptr; }
        m_status = Error;
        emit errorOccurred(m_lastError);
        return false;
    }

    m_running = true;
    m_status = Running;
    m_subThread = std::thread(&RedisAdapter::subscriptionLoop, this);

    emit statusChanged(m_status);
    redisLog(QStringLiteral("started, host=%1 port=%2 channel=%3")
             .arg(m_host).arg(m_port).arg(m_channel));
    return true;
}

bool RedisAdapter::stop()
{
    if (m_status == Stopped) return true;
    m_running = false;

    if (m_subThread.joinable()) m_subThread.join();
    if (m_subContext) { redisFree(m_subContext); m_subContext = nullptr; }
    m_connection.reset();

    m_status = Stopped;
    emit statusChanged(m_status);
    redisLog(QStringLiteral("stopped"));
    return true;
}

void RedisAdapter::subscriptionLoop()
{
    // 订阅频道
    redisReply *reply = static_cast<redisReply*>(
        redisCommand(m_subContext, "SUBSCRIBE %s", m_channel.toStdString().c_str()));
    if (reply) freeReplyObject(reply);

    while (m_running && m_subContext) {
        void *raw = nullptr;
        if (redisGetReply(m_subContext, &raw) != REDIS_OK) {
            if (m_subContext->err) {
                // 连接错误：短暂等待后重试由上层决定是否重连
                redisLog(QStringLiteral("redisGetReply error: ") + QString::fromUtf8(m_subContext->errstr));
                m_subContext->err = 0;
            }
            continue;
        }
        if (!raw) continue;

        redisReply *r = static_cast<redisReply*>(raw);
        // SUBSCRIBE 确认或 message：message 是 3 元素数组 [message, channel, payload]
        if (r->type == REDIS_REPLY_ARRAY && r->elements == 3 &&
            r->element[0]->str && strcmp(r->element[0]->str, "message") == 0) {

            char *payload = r->element[2]->str;
            size_t payloadLen = r->element[2]->len;

            // 1) gzip 解压（AFSIM 发布的是 gzip 压缩的 protobuf）
            std::string plain;
            if (!gunzipBuffer(payload, payloadLen, plain)) {
                // 兼容未压缩的情况：直接当 protobuf 用
                plain.assign(payload, payloadLen);
            }

            // 2) 解析 Envelope
            HTProto::Envelope env;
            if (env.ParseFromString(plain) && env.type() == "HTProto::Customize_Info") {
                HTProto::Customize_Info info;
                // payload 可能是 Any 包装，也可能直接是 Customize_Info 字节
                if (env.payload().size() > 0) {
                    // 发出原始数据（Customize_Info protobuf 字节），供录制/转发使用
                    const QByteArray rawPayload(env.payload().data(), (int)env.payload().size());
                    emit rawDataReceived(rawPayload, Protocol_Redis, RawType_CustomizeInfo);
                    info.ParseFromString(env.payload());
                }
                PlatformData p = RedisFieldMap::toPlatformData(&info);
                if (!p.id.isEmpty() && (p.lon != 0 || p.lat != 0)) {
                    emit platformReceived(p);
                }
            }
        }
        freeReplyObject(raw);
    }
}

void RedisAdapter::onPlatformReceived(const PlatformData &platform)
{
    // 主线程：强类型入口直接写入 DataManager（跳过 JSON 往返转换）
    DataManager::instance()->updatePlatformDirect(platform, Protocol_Redis);

    // 转发给态势显示系统（RTI DDS，未装 SDK 时为空操作）
    SituationForwarder::instance()->forwardPlatform(platform);
}

#endif // CPSS_ENABLE_REDIS
