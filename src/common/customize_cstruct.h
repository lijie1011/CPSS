/**
 * @file customize_cstruct.h
 * @brief AFSIM Customize_Info 的纯 C 结构体（用于跨语言/跨程序 UDP 传输）
 * @details 【临时方案】把 protobuf 解析成此 C struct 后直接转发其内存字节，
 *          第三方软件按同样的结构体定义解析（memcpy 即可）。
 *
 *          关键：#pragma pack(1) 保证 1 字节对齐、无 padding，
 *          确保不同编译器/平台的结构体大小与字段偏移完全一致。
 *
 *          结构体总大小 = 18*4 + 128 + 1 + 64 = 265 字节
 * @date 2026-11-24
 */

#ifndef CUSTOMIZE_CSTRUCT_H
#define CUSTOMIZE_CSTRUCT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct tagCustomizeInfoC {
    int32_t  id;                  // 平台ID
    uint32_t entityId;            // 实体ID
    uint32_t entityCamp;          // 阵营：1=蓝方, 2=红方
    uint32_t domain;              // 空间域：1=陆 2=空 3=水面 4=水下 5=太空
    uint32_t category;            // 类别
    int32_t  batchNumber;         // 批次号
    int32_t  height;              // 高度（米）
    int32_t  longitude;           // 经度 ×1e6
    int32_t  latitude;            // 纬度 ×1e6
    int32_t  absoluteHeading;     // 绝对航向 ×1e6（度）
    int32_t  relativeHeading;     // 相对航向 ×1e6
    int32_t  threatLevel;         // 威胁等级
    int32_t  distance;            // 距离
    int32_t  azimuth;             // 方位角
    int32_t  targetSpeed;         // 速度
    int32_t  targetType1;         // 目标类型
    int32_t  enemyFriendship;     // 敌我关系
    int32_t  civilianFriendship;  // 民用关系
    char     name[128];           // 名称（UTF-8，不足补0）
    uint8_t  isAlive;             // 是否存活
    char     elevation[64];       // 海拔（字符串）
} CustomizeInfoC;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // CUSTOMIZE_CSTRUCT_H
