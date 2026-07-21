#ifndef REDISADAPTER_H
#define REDISADAPTER_H

#include "protocoladapter.h"

class RedisAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit RedisAdapter(QObject *parent = nullptr);
    ~RedisAdapter();

    ProtocolType protocolType() const override { return Protocol_Redis; }
    QString adapterName() const override { return "Redis Adapter"; }

    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    void setHost(const QString &host);
    QString host() const { return m_host; }

    void setPort(quint16 port);
    quint16 port() const { return m_port; }

    void setChannel(const QString &channel);
    QString channel() const { return m_channel; }

private:
    AdapterStatus m_status;
    QString m_lastError;
    QString m_host;
    quint16 m_port;
    QString m_channel;
};

#endif
