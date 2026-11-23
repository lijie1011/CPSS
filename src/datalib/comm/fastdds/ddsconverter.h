/**
 * @file ddsconverter.h
 * @brief DDS 数据 ↔ 现有 CPSS 数据结构转换工具
 * @details 提供 PlatformData/SpecialEvent 与 QJsonObject 之间的双向转换。
 *          使用 JSON 作为中间格式，与 HTTP 适配器共用 DataManager 的解析逻辑。
 *          该头文件不依赖 Fast DDS，可独立编译和测试。
 * @date 2026-11-22
 */

#ifndef DDSCONVERTER_H
#define DDSCONVERTER_H

#include "dynamicdata.h"
#include <QJsonObject>

namespace DdsConverter {

/** @brief PlatformData -> QJsonObject */
CPSS_DATA_API QJsonObject platformToJson(const PlatformData &platform);

/** @brief QJsonObject -> PlatformData */
CPSS_DATA_API PlatformData jsonToPlatform(const QJsonObject &obj);

/** @brief SpecialEvent -> QJsonObject */
CPSS_DATA_API QJsonObject eventToJson(const SpecialEvent &event);

/** @brief QJsonObject -> SpecialEvent */
CPSS_DATA_API SpecialEvent jsonToEvent(const QJsonObject &obj);

/** @brief 给 JSON 加上 type 字段，包装成 DataManager 可识别的消息 */
CPSS_DATA_API QJsonObject wrapPlatformMessage(const PlatformData &platform);
CPSS_DATA_API QJsonObject wrapEventMessage(const SpecialEvent &event);

} // namespace DdsConverter

#endif // DDSCONVERTER_H
