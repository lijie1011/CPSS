#ifndef EVENTHISTORYDIALOG_H
#define EVENTHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QList>
#include "dynamicdata.h"

class EventHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EventHistoryDialog(QWidget *parent = nullptr);
    ~EventHistoryDialog();
    
    void updateHistory(const QList<SpecialEvent> &events);
    
private:
    QString eventTypeToString(SpecialEventType type);
    QColor eventTypeColor(SpecialEventType type);
    
    QTableWidget *m_tableWidget;
};

#endif