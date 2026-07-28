/**
 * @file waterdepthsetting.h
 * @brief 水深设置对话框
 * @details 该对话框用于配置海图水深显示相关的参数设置。
 * @date 2026-07-28
 */

#ifndef WATERDEPTHSETTING_H
#define WATERDEPTHSETTING_H

#include <QDialog>
#include "ui_waterdepthsetting.h"

/**
 * @class WaterDepthSetting
 * @brief 水深设置对话框类
 * @details 继承自QDialog，提供水深显示参数的配置界面
 */
class WaterDepthSetting : public QDialog
{
	Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
	WaterDepthSetting(QWidget *parent = 0);
	
	/**
	 * @brief 析构函数
	 */
	~WaterDepthSetting();

signals:
    /**
     * @brief 更新海图视图信号
     * @details 通知主窗口刷新海图显示
     */
	void updatChartView();

private slots:
    /**
     * @brief 将设置值更新到Enclib
     * @details 将界面上的水深设置应用到Enclib海图库
     */
	void updateValueToEncl();

private:
	Ui::WaterDepthSetting ui;  ///< UI设计器生成的界面对象
};

#endif