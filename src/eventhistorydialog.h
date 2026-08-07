/**
 * @file eventhistorydialog.h
 * @brief 事件历史对话框类定义
 * @details 本对话框用于显示和管理事件历史记录，以表格形式展示事件信息。
 */

#ifndef EVENTHISTORYDIALOG_H
#define EVENTHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
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
    
    QTableWidget *m_tableWidget;  ///< 事件表格控件
};

#endif
