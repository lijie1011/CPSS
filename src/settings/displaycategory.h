/**
 * @file displaycategory.h
 * @brief 显示类别设置对话框
 * @details 本对话框用于配置海图显示的各种类别选项，与Enclib库交互以控制显示。
 */

#ifndef DISPLAYSETTING_H
#define DISPLAYSETTING_H

#include "encl.h"
#include <QDialog>
#include <QWidgetList>
#include <QMainWindow>
#include <qapplication.h>
#include <QCheckBox>

namespace Ui {class DisplayCategory;};

/**
 * @class DisplayCategory
 * @brief 显示类别设置对话框类
 * @details 继承自QDialog，提供海图显示类别的可视化配置界面，与Enclib集成
 */
class DisplayCategory : public QDialog
{
	Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父界面指针
     */
    DisplayCategory(QWidget *parent = 0);
    
    /**
     * @brief 析构函数
     */
    ~DisplayCategory();

public slots:
    /**
     * @brief 确定按钮槽函数
     * @details 保存设置并关闭对话框
     */
	void slot_pbOK();
    
    /**
     * @brief 取消按钮槽函数
     * @details 取消设置并关闭对话框
     */
	void slot_pbCancel();
    
    /**
     * @brief 应用按钮槽函数
     * @details 保存设置但不关闭对话框
     */
	void slot_pbApplay();
      
    /**
     * @brief 更新复选框组
     * @details 从Enclib读取当前显示设置并更新界面
     */
	void updateCheckBoxGroup();
    
    /**
     * @brief 更新Enclib设置
     * @details 将界面设置同步到Enclib库
     */
	void updateEncoreSetting();

Q_SIGNALS:
    /**
     * @brief 更新海图视图信号
     * @details 通知主窗口刷新海图显示
     */
    void updateChartView();
    
    /**
     * @brief 更新动作状态信号
     * @details 通知其他组件更新动作状态
     */
    void updateQAcionsTatus();

private:
    Ui::DisplayCategory*  ui;                  ///< UI设计器生成的界面对象
	std::string          m_currentColorSchema; ///< 当前配色方案
	EnclDisplayCategory  m_displayCategory;    ///< Enclib显示类别对象
    std::vector<QCheckBox *> m_custumCheckBox; ///< 自定义复选框列表
};

#endif
