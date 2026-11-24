/**
 * @file idataforwarder.h
 * @brief 回放数据转发器抽象接口
 * @date 2026-11-24
 */

#ifndef IDATAFORWARDER_H
#define IDATAFORWARDER_H

#include <QByteArray>
#include <QJsonObject>
#include <QString>

/**
 * @class IDataForwarder
 * @brief 统一转发接口：DDS / UDP / TCP 实现此接口
 */
class IDataForwarder
{
public:
    virtual ~IDataForwarder() = default;

    /** @brief 用配置打开转发器（IP/端口/topic 等） */
    virtual bool open(const QJsonObject &cfg) = 0;
    /** @brief 发送一条 JSON 消息 */
    virtual bool send(const QByteArray &payload) = 0;
    /** @brief 关闭 */
    virtual void close() = 0;
    /** @brief 最后错误 */
    virtual QString lastError() const = 0;
    /** @brief 是否就绪 */
    virtual bool isReady() const = 0;
};

#endif // IDATAFORWARDER_H
