/**
 * @file httpadapter.h
 * @brief HTTP协议适配器类定义
 * @details 本类实现HTTP协议数据获取功能，通过周期性向指定URL发送请求来获取动态数据。
 * @date 2026-07-28
 */

#ifndef HTTPADAPTER_H
#define HTTPADAPTER_H

#include "protocoladapter.h"
#include "dynamicdata.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

/**
 * @class HttpAdapter
 * @brief HTTP协议适配器类
 * @details 继承自IProtocolAdapter，实现HTTP客户端功能，周期性从服务器获取数据
 */
class HttpAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit HttpAdapter(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~HttpAdapter();

    /**
     * @brief 获取协议类型
     * @return Protocol_HTTP
     */
    ProtocolType protocolType() const override { return Protocol_HTTP; }
    
    /**
     * @brief 获取适配器名称
     * @return "HTTP Adapter"
     */
    QString adapterName() const override { return "HTTP Adapter"; }

    /**
     * @brief 启动适配器
     * @return true 表示启动成功
     */
    bool start() override;
    
    /**
     * @brief 停止适配器
     * @return true 表示停止成功
     */
    bool stop() override;
    
    /**
     * @brief 获取当前状态
     * @return 当前适配器状态
     */
    AdapterStatus status() const override { return m_status; }
    
    /**
     * @brief 获取最后一个错误信息
     * @return 错误信息字符串
     */
    QString lastError() const override { return m_lastError; }

    /**
     * @brief 设置请求URL
     * @param url 请求地址
     */
    void setRequestUrl(const QString &url);
    
    /**
     * @brief 获取请求URL
     * @return 请求地址
     */
    QString requestUrl() const { return m_requestUrl; }

    /**
     * @brief 设置请求间隔
     * @param ms 间隔时间（毫秒）
     */
    void setRequestInterval(int ms);
    
    /**
     * @brief 获取请求间隔
     * @return 间隔时间（毫秒）
     */
    int requestInterval() const { return m_requestInterval; }

    /**
     * @brief 设置请求类型
     * @param type 请求类型（GET/POST等）
     */
    void setRequestType(const QString &type);
    
    /**
     * @brief 获取请求类型
     * @return 请求类型字符串
     */
    QString requestType() const { return m_requestType; }

    /**
     * @brief 设置请求数据
     * @param data POST请求数据
     */
    void setRequestData(const QByteArray &data);
    
    /**
     * @brief 获取请求数据
     * @return POST请求数据
     */
    QByteArray requestData() const { return m_requestData; }

private slots:
    /**
     * @brief 请求超时处理函数
     */
    void onRequestTimeout();
    
    /**
     * @brief 响应完成处理函数
     * @param reply 网络响应对象
     */
    void onReplyFinished(QNetworkReply *reply);
    
    /**
     * @brief 响应错误处理函数
     * @param error 网络错误类型
     */
    void onReplyError(QNetworkReply::NetworkError error);

private:
    /**
     * @brief 发送请求
     */
    void sendRequest();
    
    /**
     * @brief 解析并更新数据
     * @param data JSON数据
     * @param source 数据来源协议类型
     */
    void parseAndUpdate(const QJsonObject &data, ProtocolType source);
    
    /**
     * @brief 解析平台数据
     * @param obj JSON对象
     * @param source 数据来源协议类型
     * @return 平台数据结构体
     */
    PlatformData parsePlatform(const QJsonObject &obj, ProtocolType source);
    
    /**
     * @brief 解析事件数据
     * @param obj JSON对象
     * @return 事件数据结构体
     */
    SpecialEvent parseEvent(const QJsonObject &obj);

    QNetworkAccessManager *m_manager;  ///< 网络访问管理器
    AdapterStatus m_status;            ///< 当前状态
    QString m_lastError;               ///< 最后错误信息
    QString m_requestUrl;              ///< 请求URL
    int m_requestInterval;             ///< 请求间隔（毫秒）
    QString m_requestType;             ///< 请求类型
    QByteArray m_requestData;          ///< 请求数据
    QTimer m_requestTimer;             ///< 请求定时器
};

#endif
