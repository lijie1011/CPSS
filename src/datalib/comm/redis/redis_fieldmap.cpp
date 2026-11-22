/**
 * @file redis_fieldmap.cpp
 * @brief AFSIM Customize_Info -> CPSS PlatformData 字段映射实现
 * @date 2026-11-22
 */

#ifdef CPSS_ENABLE_REDIS   // Debug 下为空目标文件，避免 SDK 头依赖

#include "redis_fieldmap.h"
#include "redis_protocl.h" // 必须先定义 HTPROTO_API 导出宏
#include "protocol.pb.h"   // HTProto::Customize_Info
#include <cmath>

PlatformData RedisFieldMap::toPlatformData(const HTProto::Customize_Info *info)
{
    PlatformData p;

    if (!info) return p;

    // ---- 标识 ----
    p.id = QString::number(info->id());
    if (p.id == "0" && info->entityid() != 0) {
        p.id = QString::number(info->entityid());
    }
    p.name = QString::fromStdString(info->name());

    // ---- 位置（AFSIM 放大了 1e6 倍）----
    p.lon = info->longitude() / AFSIM_SCALE;
    p.lat = info->latitude() / AFSIM_SCALE;
    p.altitude = static_cast<double>(info->height());     // 米

    // ---- 航向/速度 ----
    // AFSIM absoluteheading 是 int32 度×1e6；负角在 protobuf 里为负值，
    // 归一化到 [0,360) 保证 PlatformItem 的 heading>=0 判断总能通过且朝向正确
    double heading = info->absoluteheading() / AFSIM_SCALE;
    p.heading = fmod(fmod(heading, 360.0) + 360.0, 360.0);
    p.speed = static_cast<double>(info->targetspeed());

    // ---- 阵营：按 AFSIM side 显示红蓝颜色 ----
    // AFSIM: entitycamp 1=blue(蓝方), 2=red(红方)
    // CPSS campColor 配色: Camp_Mine=红, Camp_Enemy=蓝, Camp_Friendly=橙
    // 为让"红方显示红、蓝方显示蓝"：
    //   红方(2) -> Camp_Mine(红) ; 蓝方(1) -> Camp_Enemy(蓝)
    switch (info->entitycamp()) {
    case 2:  p.camp = Camp_Mine;   break;   // 红方 -> 红色
    case 1:  p.camp = Camp_Enemy;  break;   // 蓝方 -> 蓝色
    default: p.camp = Camp_Unknown; break;
    }

    // ---- 平台类型（空间域）----
    p.type = mapPlatformType(info->targettype1(), info->domain());
    p.category = QStringLiteral("afsimentity");

    // ---- 目标ID（若 AFSIM 填了 entityid 且不同于自身）----
    if (info->entityid() != 0 && info->entityid() != static_cast<unsigned int>(info->id())) {
        p.targetId = QString::number(info->entityid());
    }

    // ---- 生存状态 ----
    p.dataStatus = info->isalive() ? DataStatus_Normal : DataStatus_Expired;

    // ---- 时间戳/来源 ----
    p.updateTime = QDateTime::currentMSecsSinceEpoch();
    p.validUntil = p.updateTime + 10000;   // 默认 10 秒有效期
    p.sourceProtocol = Protocol_Redis;

    return p;
}

QString RedisFieldMap::mapPlatformType(int targetType1, unsigned int domain)
{
    // AFSIM sendCustomize_Info 中 targettype1 的空间域映射：
    //   1(陆)->3  2(空)->0  3(水面)->1  4(水下)->2  5(太空)->4  其他->5
    // 这里映射为 CPSS 图标可识别的类型字符串（见 dynamicdata.h iconFileForType）
    switch (targetType1) {
    case 0: return QStringLiteral("aircraft");    // 空
    case 1: return QStringLiteral("surface");     // 水面舰艇
    case 2: return QStringLiteral("submarine");   // 水下
    case 3: return QStringLiteral("land");        // 陆
    case 4: return QStringLiteral("space");       // 太空
    default: break;
    }

    // 兜底：用原始 domain 判断（domain 1=陆 2=空 3=水面 4=水下 5=太空）
    switch (domain) {
    case 1: return QStringLiteral("land");
    case 2: return QStringLiteral("aircraft");
    case 3: return QStringLiteral("surface");
    case 4: return QStringLiteral("submarine");
    case 5: return QStringLiteral("space");
    default: return QStringLiteral("surface");
    }
}

#endif // CPSS_ENABLE_REDIS
