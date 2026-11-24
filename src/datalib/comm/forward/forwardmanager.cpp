/**
 * @file forwardmanager.cpp
 * @brief 转发管理器实现
 * @date 2026-11-24
 */

#include "forwardmanager.h"
#include "ddsforwarder.h"
#include "udpforwarder.h"
#include "tcpforwarder.h"

ForwardManager::ForwardManager()
{
}

ForwardManager::~ForwardManager()
{
    close();
}

std::unique_ptr<IDataForwarder> ForwardManager::createForwarder(Mode mode)
{
    switch (mode) {
    case Dds: return std::unique_ptr<IDataForwarder>(new DdsForwarder());
    case Udp: return std::unique_ptr<IDataForwarder>(new UdpForwarder());
    case Tcp: return std::unique_ptr<IDataForwarder>(new TcpForwarder());
    default:  return nullptr;
    }
}

bool ForwardManager::setMode(Mode mode, const QJsonObject &cfg, Format format)
{
    // 先关闭旧的
    if (m_forwarder) {
        m_forwarder->close();
        m_forwarder.reset();
    }
    m_mode = mode;
    m_format = format;
    if (mode == Off) return true;

    m_forwarder = createForwarder(mode);
    if (!m_forwarder) return false;
    return m_forwarder->open(cfg);
}

bool ForwardManager::send(const QByteArray &jsonPayload, const QByteArray &binaryPayload)
{
    if (m_mode == Off || !m_forwarder || !m_forwarder->isReady()) return false;
    // DDS 固定用 JSON（topic 判断依赖 JSON 的 type 字段）
    const QByteArray &payload = (m_mode == Dds || m_format == Json) ? jsonPayload : binaryPayload;
    return m_forwarder->send(payload);
}

void ForwardManager::close()
{
    if (m_forwarder) {
        m_forwarder->close();
        m_forwarder.reset();
    }
    m_mode = Off;
}
