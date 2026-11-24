/**
 * @file redis_fieldmap.h
 * @brief AFSIM Customize_Info -> CPSS PlatformData 字段映射
 * @details AFSIM redis_ext 通过 sendCustomize_Info 发布 HTProto::Customize_Info，
 *          经 Redis 频道 "target:data:channel" 到达。本模块负责把它的 21 个字段
 *          映射成 CPSS 内部数据结构，关键换算：
 *            - 经度/纬度/航向：AFSIM 放大了 1e6 倍(BEISHU)，这里除回去
 *            - 阵营：entitycamp 1=蓝(友) 2=红(敌) -> CampType
 *            - 空间域 targettype1 -> CPSS 平台类型字符串（用于图标匹配）
 * @date 2026-11-22
 */

#ifndef REDIS_FIELDMAP_H
#define REDIS_FIELDMAP_H

#include "dynamicdata.h"
#include "customize_cstruct.h"

// AFSIM protobuf 消息类型（redis_protocl.h 必须先于 protocol.pb.h）
#include "redis_protocl.h"
#include "protocol.pb.h"

namespace RedisFieldMap {

/** @brief AFSIM 的经纬度/航向放大倍数 */
static const double AFSIM_SCALE = 1000000.0;

/**
 * @brief 把 Customize_Info 原始 protobuf 字节解析为 PlatformData
 * @details 封装 protobuf 依赖，供回放插件等外部模块调用（无需直接 include protocol.pb.h）
 * @param raw Customize_Info 序列化字节
 * @param out 输出 PlatformData
 * @return true 解析成功
 */
CPSS_DATA_API bool parseCustomizeInfo(const QByteArray &raw, PlatformData &out);

/**
 * @brief 【临时方案】把 Customize_Info protobuf 字节解析为纯 C 结构体
 * @details 用于转发给第三方软件（按 CustomizeInfoC 定义 memcpy 解析，无需 protobuf）
 * @param raw Customize_Info protobuf 字节
 * @param out 输出 C 结构体（1字节对齐，无 padding）
 * @return true 解析成功
 */
CPSS_DATA_API bool parseCustomizeInfoC(const QByteArray &raw, CustomizeInfoC &out);

/**
 * @brief 把 AFSIM Customize_Info 映射为 CPSS PlatformData
 * @param info AFSIM 消息
 * @return PlatformData（sourceProtocol = Protocol_Redis）
 */
PlatformData toPlatformData(const HTProto::Customize_Info *info);

/**
 * @brief 空间域/类型码 -> CPSS 平台类型字符串
 * @param targetType1 AFSIM targettype1（空间域映射后的码）
 * @param domain AFSIM domain（原始空间域）
 * @return 用于图标匹配的类型字符串
 */
QString mapPlatformType(int targetType1, unsigned int domain);

} // namespace RedisFieldMap

#endif // REDIS_FIELDMAP_H
