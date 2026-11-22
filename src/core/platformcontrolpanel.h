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
#include <QComboBox>
#include <QLabel>

#include "displaystate.h"
#include "dynamicdata.h"

/**
 * @enum ApplyScope
 * @brief 批量勾选的作用范围
 */
enum class ApplyScope {
    All,      ///< 所有平台
    Red,      ///< 红方（Camp_Mine）
    Blue,     ///< 蓝方（Camp_Enemy）
    Single    ///< 仅单个平台（默认，保持原行为）
};

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

    /**
     * @brief 作用范围下拉框切换处理
     * @param index 下拉框索引
     */
    void onScopeChanged(int index);

    /**
     * @brief 树当前选中项变化（用于"单个平台"范围确定目标）
     * @param current 当前项
     * @param previous 上一个项
     */
    void onTreeCurrentChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

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
     * @brief 判断树节点是否为"设置项"（其父节点为平台节点）
     * @param item 树节点
     * @return 是设置项返回 true
     */
    bool isSettingItem(QTreeWidgetItem *item) const;

    /**
     * @brief 获取设置项的标识键
     * @param item 设置项节点
     * @return "舰船图标"/"名称标签"/"航迹"/"传感器"/"武器"/"事件"；非设置项返回空
     */
    QString settingKey(QTreeWidgetItem *item) const;

    /**
     * @brief 获取平台节点（向上找到携带 platformId 的节点）
     * @param item 任意节点
     * @return 平台节点；找不到返回 nullptr
     */
    QTreeWidgetItem* platformItemOf(QTreeWidgetItem *item) const;

    /**
     * @brief 根据作用范围获取目标平台节点列表
     * @param scope 作用范围
     * @param clickedPlatform 被点击的平台节点
     * @return 范围内的平台节点列表
     */
    QList<QTreeWidgetItem*> platformsInScope(ApplyScope scope, QTreeWidgetItem *clickedPlatform) const;

    /**
     * @brief 在平台节点下查找指定设置项
     * @param platformItem 平台节点
     * @param key 设置项标识键
     * @return 设置项节点；找不到返回 nullptr
     */
    QTreeWidgetItem* findSettingItem(QTreeWidgetItem *platformItem, const QString &key) const;

    /**
     * @brief 应用过滤（作用范围 + 搜索文本共同决定树节点显示/隐藏）
     */
    void applyFilter();

    /**
     * @brief 阵营类型转字符串
     * @param camp 阵营类型
     * @return 阵营名称字符串
     */
    QString campToString(CampType camp);

    QTreeWidget *m_treeWidget;       ///< 树控件
    QLineEdit *m_searchEdit;         ///< 搜索输入框
    QComboBox *m_scopeCombo;         ///< 作用范围下拉框（所有/红方/蓝方/单个平台）
    ApplyScope m_applyScope;         ///< 当前作用范围
    QString m_singlePlatformId;      ///< "单个平台"范围下选中的平台ID
    QPushButton *m_applyBtn;         ///< 应用按钮
    QPushButton *m_resetBtn;         ///< 重置按钮
    bool m_updatingCheckState;       ///< 是否正在更新勾选状态
    QPushButton *m_closeBtn;         ///< 关闭按钮

    DynamicObjects m_currentData;    ///< 当前数据
};

#endif
