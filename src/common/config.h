/**
 * @file config.h
 * @brief 配置管理器类定义
 * @details 提供应用程序配置的单例管理，支持配置项的读取和保存。
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

#ifdef CPSS_COMMON_EXPORT
#   define CPSS_COMMON_API __declspec(dllexport)
#else
#   define CPSS_COMMON_API __declspec(dllimport)
#endif

/**
 * @class Config
 * @brief 配置管理器类
 * @details 使用单例模式管理应用程序配置项，通过QSettings实现配置的持久化存储。
 */
class CPSS_COMMON_API Config
{
public:
    /**
     * @brief 获取配置管理器单例实例
     * @return Config引用
     */
    static Config& instance();

    /**
     * @brief 获取ENC海图路径
     * @return ENC海图目录路径
     */
    QString getChartPath() const;
    
    /**
     * @brief 设置ENC海图路径
     * @param path ENC海图目录路径
     */
    void setChartPath(const QString &path);

    /**
     * @brief获取Web服务端口
     * @return 端口号
     */
    int getWebPort() const;
    
    /**
     * @brief 设置Web服务端口
     * @param port 端口号
     */
    void setWebPort(int port);

    /**
     * @brief 获取是否自动加载ENC海图
     * @return 启用自动加载返回true
     */
    bool getAutoLoadCharts() const;
    
    /**
     * @brief 设置是否自动加载ENC海图
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
     * @brief 获取 Redis 服务器地址
     * @return IP 地址字符串（默认 127.0.0.1）
     */
    QString getRedisHost() const;

    /**
     * @brief 设置 Redis 服务器地址
     */
    void setRedisHost(const QString &host);

    /**
     * @brief 获取 Redis 端口（默认 6379）
     */
    int getRedisPort() const;

    /**
     * @brief 设置 Redis 端口
     */
    void setRedisPort(int port);

    // ---- PGDDS 配置 ----
    /** @brief PGDDS 是否启用（默认 false） */
    bool getPgDdsEnabled() const;
    void setPgDdsEnabled(bool enabled);

    /** @brief PGDDS Domain ID（默认 0） */
    int getPgDdsDomainId() const;
    void setPgDdsDomainId(int domainId);

    /** @brief PGDDS 绑定 IP（默认 127.0.0.1） */
    QString getPgDdsBindAddress() const;
    void setPgDdsBindAddress(const QString &ip);

    /** @brief PGDDS 平台数据 Topic（默认 cpss/platform） */
    QString getPgDdsPlatformTopic() const;
    void setPgDdsPlatformTopic(const QString &topic);

    /** @brief PGDDS 事件 Topic（默认 cpss/event） */
    QString getPgDdsEventTopic() const;
    void setPgDdsEventTopic(const QString &topic);

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
