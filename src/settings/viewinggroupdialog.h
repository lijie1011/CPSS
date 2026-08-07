/**
 * @file viewinggroupdialog.h
 * @brief 显示分组设置对话框
 * @details 本对话框用于配置海图显示分组的显示设置，支持基础/标准/其他三种显示分组类型。
 */

#ifndef VIEWINGGROUPDIALOG_H
#define VIEWINGGROUPDIALOG_H

#include <QDialog>
#include <map>

class QTreeWidgetItem;

namespace Ui {
class ViewingGroupDialog;
}

/**
 * @class ViewingGroupDialog
 * @brief 显示分组设置对话框类
 * @details 继承自QDialog，提供显示分组显示配置界面
 */
class ViewingGroupDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父界面指针
     */
    explicit ViewingGroupDialog(QWidget *parent = NULL);
    
    /**
     * @brief 析构函数
     */
    ~ViewingGroupDialog();

    /**
     * @brief 初始化对话框
     * @details 初始化界面和显示分组数据
     */
    void init();

public slots:
    /**
     * @brief 更新UI界面
     * @details 从Enclib读取当前设置并更新界面
     */
    void updateUI();
    
    /**
     * @brief 更新分组项
     * @param secondaryitem 子树节点项
     */
    void updateGroupIem(QTreeWidgetItem* secondaryitem);
    
    /**
     * @brief 树节点状态变更处理
     * @param item 变更的树节点
     * @param column 列索引
     */
    void treeItemChanged(QTreeWidgetItem* item, int column);

signals:
    /**
     * @brief 更新海图视图信号
     * @details 通知主窗口刷新海图显示
     */
    void updateChartView();

private:
    Ui::ViewingGroupDialog *ui;  ///< UI设计器生成的界面对象

    std::map<int,bool> baseViewGroup;      ///< 基础显示分组状态映射
    std::map<int,bool> standarViewGroup;   ///< 标准显示分组状态映射
    std::map<int,bool> otherViewGroup;     ///< 其他显示分组状态映射

    bool     firstLoad;  ///< 是否首次加载
};

#endif
