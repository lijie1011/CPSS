/**
 * @file config.cpp
 * @brief 配置管理类实现
 * @details 该类采用单例模式，负责管理应用程序的配置参数，支持从配置文件读取和保存配置。
 *          配置文件格式为INI，存储在应用程序运行目录下的cpss.ini文件中。
 * @date 2026-07-28
 */

#include "config.h"
#include <QCoreApplication>

/**
 * @brief 构造函数
 * @details 初始化配置文件路径并加载配置
 */
Config::Config()
    : m_settings(QCoreApplication::applicationDirPath() + "/cpss.ini",
                 QSettings::IniFormat)
{
    load();
}

/**
 * @brief 析构函数
 * @details 保存配置到文件
 */
Config::~Config()
{
    save();
}

/**
 * @brief 获取配置类单例实例
 * @return Config引用
 */
Config& Config::instance()
{
    static Config instance;
    return instance;
}

/**
 * @brief 获取海图数据路径
 * @return 海图数据路径字符串
 */
QString Config::getChartPath() const
{
    return m_settings.value("Chart/Path", "3dParty/Enclib").toString();
}

/**
 * @brief 设置海图数据路径
 * @param path 海图数据路径
 */
void Config::setChartPath(const QString &path)
{
    m_settings.setValue("Chart/Path", path);
}

/**
 * @brief 获取Web服务端口号
 * @return 端口号
 */
int Config::getWebPort() const
{
    return m_settings.value("Web/Port", 12345).toInt();
}

/**
 * @brief 设置Web服务端口号
 * @param port 端口号
 */
void Config::setWebPort(int port)
{
    m_settings.setValue("Web/Port", port);
}

/**
 * @brief 获取是否自动加载海图
 * @return true表示自动加载，false表示手动加载
 */
bool Config::getAutoLoadCharts() const
{
    return m_settings.value("Chart/AutoLoad", true).toBool();
}

/**
 * @brief 设置是否自动加载海图
 * @param enabled true表示自动加载，false表示手动加载
 */
void Config::setAutoLoadCharts(bool enabled)
{
    m_settings.setValue("Chart/AutoLoad", enabled);
}

/**
 * @brief 获取配色方案
 * @return 配色方案名称(DAY/NIGHT)
 */
QString Config::getColorScheme() const
{
    return m_settings.value("Display/ColorScheme", "DAY").toString();
}

/**
 * @brief 设置配色方案
 * @param scheme 配色方案名称(DAY/NIGHT)
 */
void Config::setColorScheme(const QString &scheme)
{
    m_settings.setValue("Display/ColorScheme", scheme);
}

/**
 * @brief 获取显示分类
 * @return 显示分类名称
 */
QString Config::getDisplayCategory() const
{
    return m_settings.value("Display/Category", "STANDARD").toString();
}

/**
 * @brief 设置显示分类
 * @param category 显示分类名称
 */
void Config::setDisplayCategory(const QString &category)
{
    m_settings.setValue("Display/Category", category);
}

/**
 * @brief 加载配置
 * @details 从配置文件加载所有配置项
 */
void Config::load()
{
}

/**
 * @brief 保存配置
 * @details 将所有配置项保存到配置文件
 */
void Config::save()
{
    m_settings.sync();
}
