/**
 * @file eventhistorydialog.cpp
 * @brief 事件历史对话框实现
 * @details 本类提供事件历史记录的可视化显示，以表格形式展示所有事件的
 *          详细信息，包括时间、ID、类型、名称、目标、来源、位置和描述。
 */

#include "eventhistorydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QScrollBar>
#include <QSet>

/**
 * @brief 构造函数
 * @param parent 父窗口
 */
EventHistoryDialog::EventHistoryDialog(QWidget *parent)
    : QDialog(parent),
      m_filterCombo(nullptr),
      m_tableWidget(nullptr)
{
    setWindowTitle(QStringLiteral("事件历史"));
    setMinimumSize(800, 500);
    
    // 顶部筛选栏：标签 + 平台ID下拉框（按目标/来源去重）
    QLabel *filterLabel = new QLabel(QStringLiteral("平台ID筛选："), this);
    m_filterCombo = new QComboBox(this);
    m_filterCombo->setMinimumWidth(160);
    m_filterCombo->addItem(QStringLiteral("全部"));
    connect(m_filterCombo, &QComboBox::currentTextChanged,
            this, &EventHistoryDialog::onFilterChanged);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterCombo);
    filterLayout->addStretch();

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
    layout->addLayout(filterLayout);
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
 * @details 缓存全量事件，刷新平台ID下拉框选项，并按当前选中的筛选条件
 *          重建表格。筛选逻辑在本地缓存上进行，切换下拉框无需重新取数。
 */
void EventHistoryDialog::updateHistory(const QList<SpecialEvent> &events)
{
    m_allEvents = events;

    // 先刷新下拉框（会尽量保留当前选中项），再按当前筛选条件重建表格
    refreshFilterOptions();
    rebuildTable(m_filterCombo->currentText());
}

/**
 * @brief 平台ID筛选下拉框选择变化的响应槽
 * @param platformId 当前选中的平台ID
 */
void EventHistoryDialog::onFilterChanged(const QString &platformId)
{
    rebuildTable(platformId);
}

/**
 * @brief 依据当前全量事件，刷新平台ID下拉框选项
 * @details 下拉框固定包含两个特殊项：
 *          - “全部”：显示所有事件；
 *          - “无目标(地理标绘)”：仅显示 targetId 与 sourceId 均为空的地理标绘事件；
 *          其后为按目标/来源去重排序的平台ID。
 *          仅当存在无目标事件时才追加“无目标(地理标绘)”项。
 */
void EventHistoryDialog::refreshFilterOptions()
{
    // 记住当前选中项，刷新后尽量恢复
    const QString previous = m_filterCombo->currentText();

    // 从所有事件的 targetId/sourceId 去重收集平台ID；同时统计是否存在无目标事件
    QSet<QString> idSet;
    bool hasNoTarget = false;
    for (const SpecialEvent &event : m_allEvents) {
        if (!event.targetId.isEmpty()) {
            idSet.insert(event.targetId);
        }
        if (!event.sourceId.isEmpty()) {
            idSet.insert(event.sourceId);
        }
        // 目标与来源均为空视为“无目标(地理标绘)”事件
        if (event.targetId.isEmpty() && event.sourceId.isEmpty()) {
            hasNoTarget = true;
        }
    }

    QStringList ids = idSet.toList();
    ids.sort();

    // 重建下拉框：首项固定为“全部”，若存在无目标事件则追加“无目标(地理标绘)”，其后为排序后的平台ID
    m_filterCombo->blockSignals(true);   // 重建期间屏蔽信号，避免触发多次刷新
    m_filterCombo->clear();
    m_filterCombo->addItem(QStringLiteral("全部"));
    if (hasNoTarget) {
        m_filterCombo->addItem(QStringLiteral("无目标(地理标绘)"));
    }
    m_filterCombo->addItems(ids);

    // 恢复之前的选中项；若该项已不存在（如平台已删除、无目标事件已清空），回退到“全部”
    int idx = m_filterCombo->findText(previous);
    m_filterCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    m_filterCombo->blockSignals(false);
}

/**
 * @brief 按当前筛选条件重建表格
 * @param platformId 筛选条件：
 *        - 空或“全部”：显示所有事件；
 *        - “无目标(地理标绘)”：仅显示 targetId 与 sourceId 均为空的地理标绘事件；
 *        - 其他：显示目标或来源匹配该平台ID的事件。
 */
void EventHistoryDialog::rebuildTable(const QString &platformId)
{
    const bool showAll = platformId.isEmpty() || platformId == QStringLiteral("全部");
    const bool showNoTarget = (platformId == QStringLiteral("无目标(地理标绘)"));

    m_tableWidget->setRowCount(0);
    
    for (const SpecialEvent &event : m_allEvents) {
        if (showNoTarget) {
            // 仅显示目标与来源均为空的地理标绘事件
            if (!event.targetId.isEmpty() || !event.sourceId.isEmpty()) {
                continue;
            }
        } else if (!showAll) {
            // 非“全部”/“无目标”时，仅显示目标或来源匹配该平台ID的事件
            if (event.targetId != platformId && event.sourceId != platformId) {
                continue;
            }
        }

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
