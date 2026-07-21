#include "redisadapter.h"
#include "logger.h"

RedisAdapter::RedisAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_status(Stopped),
      m_port(6379)
{
}

RedisAdapter::~RedisAdapter()
{
    stop();
}

bool RedisAdapter::start()
{
    m_status = Error;
    m_lastError = "Redis adapter not implemented - requires hiredis library";
    emit statusChanged(m_status);
    emit errorOccurred(m_lastError);
    Logger::warn("Redis Adapter not implemented: %s", m_lastError.toStdString().c_str());
    return false;
}

bool RedisAdapter::stop()
{
    m_status = Stopped;
    emit statusChanged(m_status);
    return true;
}

void RedisAdapter::setHost(const QString &host)
{
    m_host = host;
}

void RedisAdapter::setPort(quint16 port)
{
    m_port = port;
}

void RedisAdapter::setChannel(const QString &channel)
{
    m_channel = channel;
}
