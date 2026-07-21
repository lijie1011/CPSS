#ifndef DDSADAPTER_H
#define DDSADAPTER_H

#include "protocoladapter.h"

class DdsAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit DdsAdapter(QObject *parent = nullptr);
    ~DdsAdapter();

    ProtocolType protocolType() const override { return Protocol_DDS; }
    QString adapterName() const override { return "DDS Adapter"; }

    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    void setTopicName(const QString &topic);
    QString topicName() const { return m_topicName; }

private:
    AdapterStatus m_status;
    QString m_lastError;
    QString m_topicName;
};

#endif
