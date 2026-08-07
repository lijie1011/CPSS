/**
 * @file waterdepthsetting.cpp
 * @brief 水深设置对话框实现
 * @details 本类提供水深和等深线参数的配置界面，支持安全水深、安全等深线、
 *          深度等深线和浅水等深线的设置。
 */

#include "waterdepthsetting.h"
#include "viewwidget.h"
#include <QDoubleValidator>
#include <QString>
#include <QIcon>
#include "encl.h"

/**
 * @brief 构造函数
 * @param parent 父界面
 */
WaterDepthSetting::WaterDepthSetting(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setModal(true);
    setWindowTitle(QString::fromLocal8Bit("水深设置"));
	setWindowIcon(QIcon(":/icons/images/ruler.png"));

	ui.m_SafetyDepthLineEdit->setValidator(new QDoubleValidator(0.0,100.0,3,this));
	ui.m_SafetyContourLineEdit->setValidator(new QDoubleValidator(0.0,100.0,3,this));
	ui.m_DeepLineEdit->setValidator(new QDoubleValidator(0.0,100.0,3,this));
	ui.m_ShallowLineEdit->setValidator(new QDoubleValidator(0.0,100.0,3,this));

	double safetyDepth = EnclDrawGetSafetyDepth();
	double safetyContour = EnclDrawGetSafetyContour();
	double deepContour = EnclDrawGetDeepContour();
	double shallowContour = EnclDrawGetShallowContour();

	QString safetyContourStr = QString::number(safetyContour,'f',1);
	QString safetyDepthStr =QString::number(safetyDepth,'f',1);
	QString deepContourStr =QString::number(deepContour,'f',1);
	QString shallowContourStr =QString::number(shallowContour,'f',1);
	ui.m_SafetyDepthLineEdit->setText(safetyDepthStr);
	ui.m_SafetyContourLineEdit->setText(safetyContourStr);
	ui.m_DeepLineEdit->setText(deepContourStr);
	ui.m_ShallowLineEdit->setText(shallowContourStr);

	connect(ui.buttonBox,SIGNAL(accepted()),this,SLOT(updateValueToEncl()));
}

/**
 * @brief 析构函数
 */
WaterDepthSetting::~WaterDepthSetting()
{
}

/**
 * @brief 更新设置到Enclib
 * @details 将界面输入的水深参数应用到Enclib海图库
 */
void WaterDepthSetting::updateValueToEncl()
{
	double safetyDepth = ui.m_SafetyDepthLineEdit->text().toDouble();
	double safetyDepthContour= ui.m_SafetyContourLineEdit->text().toDouble();
	double deepContour = ui.m_DeepLineEdit->text().toDouble();
	double shallowContour = ui.m_ShallowLineEdit->text().toDouble();

	EnclDrawSetSafetyDepth(safetyDepth);
	EnclDrawSetSafetyContour(safetyDepthContour);
	EnclDrawSetShallowContour(shallowContour);
	EnclDrawSetDeepContour(deepContour);

    emit updatChartView();
}
