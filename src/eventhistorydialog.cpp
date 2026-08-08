/**
 * @file eventhistorydialog.cpp
 * @brief 事件历史对话框实现
 * @details 本类提供事件历史记录的可视化显示，以表格形式展示所有事件的
 *          详细信息，包括时间、ID、类型、名称、目标、来源、位置和描述。
 */

#include "eventhistorydialog.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QScrollBar>

/**
 * @brief 构造函数
 * @param parent 父窗口
 */
EventHistoryDialog::EventHistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("事件历史"));
    setMinimumSize(800, 500);
    
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(8);
    QStringList headers;
    headers << QStringLiteral("时间") << QStringLiteral("编号") << QStringLiteral("类型") << QStringLiteral("名称") << QStringLiteral("目标") << QStringLiteral("来源") << QStringLiteral("位置") << QStringLiteral("描述");
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_tableWidget);
}

/**
 * @brief 析构函数
 */
EventHistoryDialog::~EventHistoryDialog()
{
}

/**
 * @brief 事件类型转字符串
 * @param type 事件类型
 * @return 事件类型字符串
 */
QString EventHistoryDialog::eventTypeToString(SpecialEventType type)
{
    switch (type) {
    case Event_Attack: return QStringLiteral("攻击");
    case Event_Defense: return QStringLiteral("防御");
    case Event_Alert: return QStringLiteral("告警");
    case Event_MissionStart: return QStringLiteral("任务开始");
    case Event_MissionEnd: return QStringLiteral("任务结束");
    case Event_Contact: return QStringLiteral("接触");
    case Event_Lost: return QStringLiteral("失联");
    case Event_Damage: return QStringLiteral("损伤");
    case Event_Repair: return QStringLiteral("修理");
    case Event_Custom: return QStringLiteral("自定义");
    default: return QStringLiteral("未知");
    }
}

/**
 * @brief 获取事件类型对应的显示颜色
 * @param type 事件类型
 * @return 用于表格单元格背景的颜色值
 */
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

/**
 * @brief 更新历史记录
 * @param events 事件列表
 */
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
            location = QString("%1, %2").arg(event.lon, 0, 'f', 6).arg(event.lat, 0, 'f', 6);
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
