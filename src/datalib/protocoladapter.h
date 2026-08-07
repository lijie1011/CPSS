/**
 * @file protocoladapter.h
 * @brief 协议适配器接口定义
 * @details 定义了协议适配器的抽象接口。所有协议适配器（如HTTP适配器）
 *          必须实现此接口。
 * @date 2026-07-28
 */

#ifndef PROTOCOLADAPTER_H
#define PROTOCOLADAPTER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "dynamicdata.h"

/**
 * @class IProtocolAdapter
 * @brief 协议适配器接口类
 * @details 继承自QObject，定义了协议适配器的标准接口，包括启动/停止、状态查询等功能
 */
class CPSS_DATA_API IProtocolAdapter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit IProtocolAdapter(QObject *parent = nullptr) : QObject(parent) {}
    
    /**
     * @brief 虚析构函数
     */
    virtual ~IProtocolAdapter() {}

    /**
     * @brief 获取协议类型
     * @return 协议类型枚举值
     */
    virtual ProtocolType protocolType() const = 0;
    
    /**
     * @brief 获取适配器名称
     * @return 适配器名称
     */
    virtual QString adapterName() const = 0;

    /**
     * @enum AdapterStatus
     * @brief 适配器状态枚举
     * @details 描述协议适配器的运行状态，用于外部查询和信号通知
     */
    enum AdapterStatus {
        Stopped,    ///< 已停止
        Starting,   ///< 启动中
        Running,    ///< 运行中
        Error       ///< 错误状态
    };

    /**
     * @brief 启动适配器
     * @return true 表示启动成功
     */
    virtual bool start() = 0;
    
    /**
     * @brief 停止适配器
     * @return true 表示停止成功
     */
    virtual bool stop() = 0;
    
    /**
     * @brief 获取当前状态
     * @return 当前状态
     */
    virtual AdapterStatus status() const = 0;
    
    /**
     * @brief 获取最后一个错误信息
     * @return 错误信息字符串
     */
    virtual QString lastError() const = 0;

    /**
     * @brief 设置数据有效期
     * @param ms 有效期（毫秒）
     */
    virtual void setValidDuration(qint64 ms) { m_validDuration = ms; }
    
    /**
     * @brief 获取数据有效期
     * @return 有效期（毫秒）
     */
    qint64 validDuration() const { return m_validDuration; }

signals:
    /**
     * @brief 数据接收信号
     * @param data 接收到的JSON数据
     * @param source 数据来源协议类型
     */
    void dataReceived(const QJsonObject &data, ProtocolType source);
    
    /**
     * @brief 状态变更信号
     * @param status 新的状态
     */
    void statusChanged(AdapterStatus status);
    
    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

protected:
    qint64 m_validDuration = 5000;  ///< 数据有效期（毫秒），默认5秒
};

#endif
