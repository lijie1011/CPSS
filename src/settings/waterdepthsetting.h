#ifndef WATERDEPTHSETTING_H
#define WATERDEPTHSETTING_H

#include <QDialog>
#include "ui_waterdepthsetting.h"

class WaterDepthSetting : public QDialog
{
	Q_OBJECT

public:
	WaterDepthSetting(QWidget *parent = 0);
	~WaterDepthSetting();

signals:
	void updatChartView();

private slots:
	void updateValueToEncl();

private:
	Ui::WaterDepthSetting ui;
};

#endif