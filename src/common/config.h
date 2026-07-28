/**
 * @file config.h
 * @brief 配置管理器类定义
 * @details 提供应用程序配置的单例管理，支持配置项的读取和保存。
 * @date 2026-07-28
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

/**
 * @class Config
 * @brief 配置管理器类
 * @details 采用单例模式，管理应用程序的配置项，使用QSettings进行配置的持久化存储
 */
class Config
{
public:
    /**
     * @brief 获取配置管理器单例实例
     * @return Config引用
     */
    static Config& instance();

    /**
     * @brief 获取海图路径
     * @return 海图目录路径
     */
    QString getChartPath() const;
    
    /**
     * @brief 设置海图路径
     * @param path 海图目录路径
     */
    void setChartPath(const QString &path);

    /**
     * @brief 获取Web服务端口
     * @return 端口号
     */
    int getWebPort() const;
    
    /**
     * @brief 设置Web服务端口
     * @param port 端口号
     */
    void setWebPort(int port);

    /**
     * @brief 获取是否自动加载海图
     * @return 自动加载返回true
     */
    bool getAutoLoadCharts() const;
    
    /**
     * @brief 设置是否自动加载海图
     * @param enabled 是否启用自动加载
     */
    void setAutoLoadCharts(bool enabled);

    /**
     * @brief 获取配色方案
     * @return 配色方案名称
     */
    QString getColorScheme() const;
    
    /**
     * @brief 设置配色方案
     * @param scheme 配色方案名称
     */
    void setColorScheme(const QString &scheme);

    /**
     * @brief 获取显示类别
     * @return 显示类别名称
     */
    QString getDisplayCategory() const;
    
    /**
     * @brief 设置显示类别
     * @param category 显示类别名称
     */
    void setDisplayCategory(const QString &category);

    /**
     * @brief 加载配置
     * @details 从配置文件读取配置项
     */
    void load();
    
    /**
     * @brief 保存配置
     * @details 将配置项写入配置文件
     */
    void save();

private:
    /**
     * @brief 私有构造函数
     */
    Config();
    
    /**
     * @brief 私有析构函数
     */
    ~Config();

    QSettings m_settings;  ///< 配置存储对象
};

#endif
