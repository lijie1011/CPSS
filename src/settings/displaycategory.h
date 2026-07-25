#ifndef DISPLAYSETTING_H
#define DISPLAYSETTING_H

#include "encl.h"
#include <QDialog>
#include <QWidgetList>
#include <QMainWindow>
#include <qapplication.h>
#include <QCheckBox>

namespace Ui {class DisplayCategory;};

class DisplayCategory : public QDialog
{
	Q_OBJECT

public:
    DisplayCategory(QWidget *parent = 0);
    ~DisplayCategory();


public slots:
	void slot_pbOK();
	void slot_pbCancel();
	void slot_pbApplay();
      
	void updateCheckBoxGroup();
	void updateEncoreSetting();

Q_SIGNALS:
    void updateChartView();
    void updateQAcionsTatus();

private:
    Ui::DisplayCategory*  ui;
	std::string          m_currentColorSchema;
	EnclDisplayCategory  m_displayCategory;
    std::vector<QCheckBox *> m_custumCheckBox;
};

#endif