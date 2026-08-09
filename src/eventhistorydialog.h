/**
 * @file eventhistorydialog.h
 * @brief 事件历史对话框类定义
 * @details 本对话框用于显示和管理事件历史记录，以表格形式展示事件信息。
 */

#ifndef EVENTHISTORYDIALOG_H
#define EVENTHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include <QList>
#include "dynamicdata.h"

/**
 * @class EventHistoryDialog
 * @brief 事件历史对话框类
 * @details 继承自 QDialog，提供事件历史记录的显示界面
 */
class EventHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit EventHistoryDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~EventHistoryDialog();
    
    /**
     * @brief 更新事件历史
     * @param events 事件列表
     */
    void updateHistory(const QList<SpecialEvent> &events);
    
private slots:
    /**
     * @brief 平台ID筛选下拉框选择变化时刷新表格
     * @param platformId 当前选中的平台ID（"全部"表示不筛选）
     */
    void onFilterChanged(const QString &platformId);

private:
    /**
     * @brief 事件类型转字符串
     * @param type 事件类型枚举值
     * @return 事件类型名称字符串
     */
    QString eventTypeToString(SpecialEventType type);
    
    /**
     * @brief 根据事件类型获取颜色
     * @param type 事件类型枚举值
     * @return 对应事件类型的颜色
     */
    QColor eventTypeColor(SpecialEventType type);
    
    /**
     * @brief 按当前筛选条件重建表格
     * @param platformId 平台ID；为空或"全部"时显示所有事件，
     *        否则只显示 targetId 或 sourceId 匹配该ID的事件
     */
    void rebuildTable(const QString &platformId);
    
    /**
     * @brief 依据当前全量事件，刷新平台ID下拉框选项
     * @details 从所有事件的 targetId/sourceId 去重收集平台ID，
     *          排序后填充下拉框，并尽量保留当前选中项。
     */
    void refreshFilterOptions();
    
    QComboBox *m_filterCombo;             ///< 平台ID筛选下拉框（目标/来源去重）
    QTableWidget *m_tableWidget;          ///< 事件表格控件
    QList<SpecialEvent> m_allEvents;      ///< 全量事件缓存，供筛选时复用
};

#endif
