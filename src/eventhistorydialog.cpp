#include "eventhistorydialog.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QScrollBar>

EventHistoryDialog::EventHistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Event History");
    setMinimumSize(800, 500);
    
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(8);
    QStringList headers;
    headers << "Time" << "ID" << "Type" << "Name" << "Target" << "Source" << "Location" << "Description";
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_tableWidget);
}

EventHistoryDialog::~EventHistoryDialog()
{
}

QString EventHistoryDialog::eventTypeToString(SpecialEventType type)
{
    switch (type) {
    case Event_Attack: return "Attack";
    case Event_Defense: return "Defense";
    case Event_Alert: return "Alert";
    case Event_MissionStart: return "Mission Start";
    case Event_MissionEnd: return "Mission End";
    case Event_Contact: return "Contact";
    case Event_Lost: return "Lost";
    case Event_Damage: return "Damage";
    case Event_Repair: return "Repair";
    case Event_Custom: return "Custom";
    default: return "Unknown";
    }
}

QColor EventHistoryDialog::eventTypeColor(SpecialEventType type)
{
    switch (type) {
    case Event_Attack: return QColor(139, 0, 0);
    case Event_Defense: return QColor(0, 0, 255);
    case Event_Alert: return QColor(255, 0, 0);
    case Event_MissionStart: return QColor(0, 128, 0);
    case Event_MissionEnd: return QColor(128, 128, 128);
    case Event_Contact: return QColor(0, 255, 255);
    case Event_Lost: return QColor(255, 0, 255);
    case Event_Damage: return QColor(255, 165, 0);
    case Event_Repair: return QColor(144, 238, 144);
    case Event_Custom: return QColor(139, 0, 139);
    default: return QColor(128, 128, 128);
    }
}

void EventHistoryDialog::updateHistory(const QList<SpecialEvent> &events)
{
    m_tableWidget->setRowCount(0);
    
    for (const SpecialEvent &event : events) {
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);
        
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(event.timestamp);
        QTableWidgetItem *timeItem = new QTableWidgetItem(timestamp.toString("yyyy-MM-dd HH:mm:ss"));
        timeItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 0, timeItem);
        
        QTableWidgetItem *idItem = new QTableWidgetItem(event.eventId);
        idItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 1, idItem);
        
        QTableWidgetItem *typeItem = new QTableWidgetItem(eventTypeToString(event.eventType));
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setBackgroundColor(eventTypeColor(event.eventType));
        typeItem->setForeground(Qt::white);
        m_tableWidget->setItem(row, 2, typeItem);
        
        QTableWidgetItem *nameItem = new QTableWidgetItem(event.eventName);
        m_tableWidget->setItem(row, 3, nameItem);
        
        QString target = event.targetId.isEmpty() ? "-" : event.targetId;
        QTableWidgetItem *targetItem = new QTableWidgetItem(target);
        targetItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 4, targetItem);
        
        QString source = event.sourceId.isEmpty() ? "-" : event.sourceId;
        QTableWidgetItem *sourceItem = new QTableWidgetItem(source);
        sourceItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 5, sourceItem);
        
        QString location;
        if (event.lon != 0 && event.lat != 0) {
            location = QString("%1, %2").arg(event.lon, 0, 'f', 4).arg(event.lat, 0, 'f', 4);
        } else {
            location = "-";
        }
        QTableWidgetItem *locationItem = new QTableWidgetItem(location);
        locationItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(row, 6, locationItem);
        
        QTableWidgetItem *descItem = new QTableWidgetItem(event.description);
        m_tableWidget->setItem(row, 7, descItem);
    }
    
    m_tableWidget->resizeColumnsToContents();
}