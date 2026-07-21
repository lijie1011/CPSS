#include "ddsadapter.h"
#include "logger.h"

DdsAdapter::DdsAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_status(Stopped)
{
}

DdsAdapter::~DdsAdapter()
{
    stop();
}

bool DdsAdapter::start()
{
    m_status = Error;
    m_lastError = "DDS adapter not implemented - requires external DDS library (e.g., FastDDS, OpenDDS)";
    emit statusChanged(m_status);
    emit errorOccurred(m_lastError);
    Logger::warn("DDS Adapter not implemented: %s", m_lastError.toStdString().c_str());
    return false;
}

bool DdsAdapter::stop()
{
    m_status = Stopped;
    emit statusChanged(m_status);
    return true;
}

void DdsAdapter::setTopicName(const QString &topic)
{
    m_topicName = topic;
}
