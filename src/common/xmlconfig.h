/**
 * @file xmlconfig.h
 * @brief XML配置管理器类定义
 * @details 单例模式，读取/写入cpss.xml，提供每个插件的关闭模式
 *          并带有全局默认值回退。同时维护自动生成的插件清单，
 *          列出当前加载的所有插件（id、名称、版本）。
 */

#ifndef XMLCONFIG_H
#define XMLCONFIG_H

#include <QString>
#include <QMap>
#include <QList>

#ifdef CPSS_COMMON_EXPORT
#   define CPSS_COMMON_API __declspec(dllexport)
#else
#   define CPSS_COMMON_API __declspec(dllimport)
#endif

class CPSS_COMMON_API XmlConfig
{
public:
    struct PluginInfo {
        QString id;
        QString name;
        QString version;
    };

    static XmlConfig& instance();

    int pluginCloseMode(const QString &pluginId) const;
    int defaultPluginCloseMode() const;
    void setPluginCloseMode(const QString &pluginId, int mode);
    void setDefaultPluginCloseMode(int mode);

    void updatePluginManifest(const QList<PluginInfo> &plugins);
    QList<PluginInfo> pluginManifest() const;

    void load();
    void save();

private:
    XmlConfig();
    ~XmlConfig();

    QString m_filePath;
    int m_defaultCloseMode;
    QMap<QString, int> m_pluginCloseModes;
    QList<PluginInfo> m_pluginManifest;
};

#endif
