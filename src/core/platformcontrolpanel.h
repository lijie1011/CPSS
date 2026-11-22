/**
 * @file platformcontrolpanel.h
 * @brief 平台控制面板对话框类定义
 * @details 本对话框提供平台显示状态的可视化控制界面，支持
 *          按阵营对平台列表分组。用户可勾选/取消勾选以控制
 *          平台、传感器、武器及航迹的可见性。
 */

#ifndef PLATFORMCONTROLPANEL_H
#define PLATFORMCONTROLPANEL_H

#include <QDialog>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QCheckBox>

#include "displaystate.h"
#include "dynamicdata.h"

/**
 * @class PlatformControlPanel
 * @brief 平台控制面板对话框类
 * @details 继承自 QDialog，提供平台显示状态的配置界面
 */
class PlatformControlPanel : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit PlatformControlPanel(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~PlatformControlPanel();

    /**
     * @brief 使用数据初始化对话框
     * @param data 动态目标数据
     * @param currentStates 当前显示状态映射
     */
    void initWithData(const DynamicObjects &data, const DisplayStateMap &currentStates = DisplayStateMap());

signals:
    /**
     * @brief 显示状态已改变信号
     * @param stateMap 更新后的显示状态映射
     */
    void displayStateChanged(const DisplayStateMap &stateMap);

private slots:
    /**
     * @brief 应用按钮点击处理函数
     */
    void onApplyClicked();
    
    /**
     * @brief 重置按钮点击处理函数
     */
    void onResetClicked();
    
    /**
     * @brief 关闭按钮点击处理函数
     */
    void onCloseClicked();
    
    /**
     * @brief 搜索文本变化处理函数
     * @param text 搜索文本
     */
    void onSearchTextChanged(const QString &text);
    
    /**
     * @brief 树节点点击处理函数
     * @param item 被点击的树节点
     * @param column 列索引
     */
    void onTreeItemClicked(QTreeWidgetItem *item, int column);

private:
    /**
     * @brief 初始化界面
     */
    void initUI();
    
    /**
     * @brief 填充树控件
     * @param data 动态目标数据
     * @param currentStates 当前显示状态映射
     */
    void populateTree(const DynamicObjects &data, const DisplayStateMap &currentStates = DisplayStateMap());
    
    /**
     * @brief 添加平台节点
     * @param campItem 阵营父节点
     * @param platform 平台数据
     * @param state 显示状态
     */
    void addPlatformNode(QTreeWidgetItem *campItem, const PlatformData &platform, const PlatformDisplayState &state = PlatformDisplayState());
    
    /**
     * @brief 收集显示状态
     * @param stateMap 输出显示状态映射
     */
    void collectStates(DisplayStateMap &stateMap);

    /**
     * @brief 阵营类型转字符串
     * @param camp 阵营类型
     * @return 阵营名称字符串
     */
    QString campToString(CampType camp);

    QTreeWidget *m_treeWidget;       ///< 树控件
    QLineEdit *m_searchEdit;         ///< 搜索输入框
    QPushButton *m_applyBtn;         ///< 应用按钮
    QPushButton *m_resetBtn;         ///< 重置按钮
    bool m_updatingCheckState;       ///< 是否正在更新勾选状态
    QPushButton *m_closeBtn;         ///< 关闭按钮

    DynamicObjects m_currentData;    ///< 当前数据
};

#endif
