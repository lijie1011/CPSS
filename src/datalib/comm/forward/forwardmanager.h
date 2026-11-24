/**
 * @file forwardmanager.h
 * @brief 转发管理器：单选策略（DDS/UDP/TCP/Off）
 * @details 根据选择的模式创建并持有唯一一个转发器，切换时关闭旧的。
 * @date 2026-11-24
 */

#ifndef FORWARDMANAGER_H
#define FORWARDMANAGER_H

#include "idataforwarder.h"
#include "dynamicdata.h"   // CPSS_DATA_API
#include <QJsonObject>
#include <QString>
#include <memory>

class CPSS_DATA_API ForwardManager
{
public:
    enum Mode { Off = 0, Dds = 1, Udp = 2, Tcp = 3 };
    enum Format { Json = 0, Binary = 1 };

    ForwardManager();
    ~ForwardManager();

    /** @brief 设置模式与格式并打开（Off 则关闭当前转发器） */
    bool setMode(Mode mode, const QJsonObject &cfg, Format format = Json);
    /** @brief 发送（根据格式自动选 json 或 binary payload） */
    bool send(const QByteArray &jsonPayload, const QByteArray &binaryPayload);
    void close();

    Mode mode() const { return m_mode; }
    Format format() const { return m_format; }
    bool isActive() const { return m_mode != Off && m_forwarder && m_forwarder->isReady(); }
    QString lastError() const { return m_forwarder ? m_forwarder->lastError() : QString(); }

private:
    std::unique_ptr<IDataForwarder> createForwarder(Mode mode);

    Mode m_mode = Off;
    Format m_format = Json;
    std::unique_ptr<IDataForwarder> m_forwarder;
};

#endif // FORWARDMANAGER_H
