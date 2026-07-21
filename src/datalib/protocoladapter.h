#ifndef PROTOCOLADAPTER_H
#define PROTOCOLADAPTER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "dynamicdata.h"

class IProtocolAdapter : public QObject
{
    Q_OBJECT

public:
    explicit IProtocolAdapter(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IProtocolAdapter() {}

    virtual ProtocolType protocolType() const = 0;
    virtual QString adapterName() const = 0;

    enum AdapterStatus {
        Stopped,
        Starting,
        Running,
        Error
    };

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual AdapterStatus status() const = 0;
    virtual QString lastError() const = 0;

    virtual void setValidDuration(qint64 ms) { m_validDuration = ms; }
    qint64 validDuration() const { return m_validDuration; }

signals:
    void dataReceived(const QJsonObject &data, ProtocolType source);
    void statusChanged(AdapterStatus status);
    void errorOccurred(const QString &error);

protected:
    qint64 m_validDuration = 5000;
};

#endif
