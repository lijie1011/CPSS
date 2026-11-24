/**
 * @file forwardconfig.h
 * @brief 转发配置读取工具（兼容 Qt5.12 不支持 QJsonObject::value(key, default)）
 * @date 2026-11-24
 */

#ifndef FORWARDCONFIG_H
#define FORWARDCONFIG_H

#include <QJsonObject>
#include <QString>

inline QString cfgStr(const QJsonObject &cfg, const QString &key, const QString &def)
{
    return cfg.contains(key) ? cfg.value(key).toString() : def;
}

inline int cfgInt(const QJsonObject &cfg, const QString &key, int def)
{
    return cfg.contains(key) ? cfg.value(key).toInt() : def;
}

inline bool cfgBool(const QJsonObject &cfg, const QString &key, bool def)
{
    return cfg.contains(key) ? cfg.value(key).toBool() : def;
}

#endif // FORWARDCONFIG_H
