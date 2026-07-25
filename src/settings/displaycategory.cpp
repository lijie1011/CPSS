#include "displaycategory.h"
#include "ui_displaycategory.h"
#include <QIcon>

DisplayCategory::DisplayCategory(QWidget *parent)
	: QDialog(parent)
{
    ui = new Ui::DisplayCategory();
	setWindowIcon(QIcon(":/icons/images/settings64x64t.png"));

	QWidget::setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);
    setModal(true);

    ui->cancelButton->setVisible(false);
    ui->okButton->setVisible(false);

    m_custumCheckBox.push_back(ui->checkBox_1);
    m_custumCheckBox.push_back(ui->checkBox_2);
    m_custumCheckBox.push_back(ui->checkBox_3);
    m_custumCheckBox.push_back(ui->checkBox_4);
    m_custumCheckBox.push_back(ui->checkBox_5);
    m_custumCheckBox.push_back(ui->checkBox_6);
    m_custumCheckBox.push_back(ui->checkBox_7);
    m_custumCheckBox.push_back(ui->checkBox_8);
    m_custumCheckBox.push_back(ui->checkBox_9);
    m_custumCheckBox.push_back(ui->checkBox_10);
    m_custumCheckBox.push_back(ui->checkBox_11);
    m_custumCheckBox.push_back(ui->checkBox_12);
    m_custumCheckBox.push_back(ui->checkBox_13);
    m_custumCheckBox.push_back(ui->checkBox_14);
    m_custumCheckBox.push_back(ui->checkBox_15);
    m_custumCheckBox.push_back(ui->checkBox_16);
    m_custumCheckBox.push_back(ui->checkBox_17);

    if ( true == EnclDrawGetShowTwoShades())
	{
		ui->twoShaderSchem->setChecked(true);
	}else
	{
		ui->fourShaderScheme->setChecked(true);
	}

    const char* pColorScheme= EnclDrawGetColorScheme();
    if(strcmp(pColorScheme, "DAY") == 0){
        ui->colorSchemDay->setChecked(true);
    }else if (strcmp(pColorScheme, "DUSK") == 0)
    {
        ui->colorSchemDusk->setChecked(true);
    }else if (strcmp(pColorScheme, "NIGHT") == 0)
	{
        ui->colorSchemNight->setChecked(true);
    }

    int mode = EnclDrawGetMonoView();
    if(0 == mode){
        ui->normal->setChecked(true);
    }
    else if(-1 == mode)
    {
        ui->whiteChartBlackLine->setChecked(true);
    }else if(1 == mode)
    {
        ui->blackChartWhiteLine->setChecked(true);
    }

    ui->satandardGroupBox->setEnabled(false);
    ui->otherGroupBox->setEnabled(false);

    EnclDisplayCategory encldc =EnclDrawGetDisplayCategory();
    if (ENCL_BASE == encldc)
    {
       ui->baseDisplay->setChecked(true);
    }if (ENCL_STANDARD == encldc)
    {
       ui->standardDisplay->setChecked(true);
    }
    if (ENCL_OTHER == encldc )
    {
       ui->otherDisplay->setChecked(true);
    }if (ENCL_CUSTOM ==  encldc)
    {
        ui->custumDisplay->setChecked(true);
    }

    updateCheckBoxGroup();

    connect(ui->okButton,SIGNAL(clicked()),this,SLOT(slot_pbOK()));
    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(slot_pbCancel()));
    connect(ui->applyButton,SIGNAL(clicked()),this,SLOT(slot_pbApplay()));

    connect(ui->baseDisplay,SIGNAL(clicked()),this,SLOT(updateCheckBoxGroup()));
    connect(ui->standardDisplay,SIGNAL(clicked()),this,SLOT(updateCheckBoxGroup()));
    connect(ui->otherDisplay,SIGNAL(clicked()),this,SLOT(updateCheckBoxGroup()));
    connect(ui->custumDisplay,SIGNAL(clicked()),this,SLOT(updateCheckBoxGroup()));
}

DisplayCategory::~DisplayCategory()
{
	delete ui;
}

void DisplayCategory::slot_pbOK()
{
	updateEncoreSetting();
    emit updateChartView();
    emit updateQAcionsTatus();
	this->close();
}

void DisplayCategory::slot_pbCancel()
{
	this->close();
}

void DisplayCategory::slot_pbApplay()
{
	updateEncoreSetting();
    emit updateChartView();
    emit updateQAcionsTatus();
}

void DisplayCategory::updateCheckBoxGroup()
{  
    if (ui->baseDisplay->isChecked())
    {
        for (int i =0; i < m_custumCheckBox.size(); ++i)
        {
           m_custumCheckBox[i]->setChecked(false);
        }
    }

    if (ui->standardDisplay->isChecked())
    {
        for (int i =0; i < m_custumCheckBox.size(); ++i)
        {
            if(i < 10)
            {
                m_custumCheckBox[i]->setChecked(true);
            }else
            {
                m_custumCheckBox[i]->setChecked(false);
            }
        }
    }

    if (ui->otherDisplay->isChecked())
    {
        for (int i =0; i < m_custumCheckBox.size(); ++i)
        {
            m_custumCheckBox[i]->setChecked(true);
        }
    }

    if (ui->custumDisplay->isChecked())
    {
        for (int i =0; i < m_custumCheckBox.size(); ++i)
        {
            int groupLayer= i+2;
            if(i < 10)
            {
                if ( 3 == groupLayer)
                {
                    if (EnclDrawGetViewGroupLayer(31) && EnclDrawGetViewGroupLayer(32))
                    {
                        m_custumCheckBox[i]->setChecked(true);
                    }else
                       m_custumCheckBox[i]->setChecked(false);
                }else
                    true == EnclDrawGetViewGroupLayer(groupLayer) ? m_custumCheckBox[i]->setChecked(true) : m_custumCheckBox[i]->setChecked(false);
            }
         }
    }


	if(ui->baseDisplay->isChecked() || ui->standardDisplay->isChecked() || ui->otherDisplay->isChecked())
	{
	    ui->satandardGroupBox->setEnabled(false);
		ui->otherGroupBox->setEnabled(false);
		return;
	}

	ui->satandardGroupBox->setEnabled(true);
	ui->otherGroupBox->setEnabled(true);
}

void DisplayCategory::updateEncoreSetting()
{
    ui->satandardGroupBox->setEnabled(false);
    ui->otherGroupBox->setEnabled(false);

	if (ui->twoShaderSchem->isChecked())
	{
		EnclDrawSetShowTwoShades(true);
	}else if (ui->fourShaderScheme->isChecked())
	{
		EnclDrawSetShowTwoShades(false);
    }

	if (ui->colorSchemDay->isChecked())
	{
		EnclDrawSetColorScheme("DAY");
	}else if (ui->colorSchemDusk->isChecked())
	{
		EnclDrawSetColorScheme("DUSK");
	}else if (ui->colorSchemNight->isChecked())
	{
		EnclDrawSetColorScheme("NIGHT");
    }

    if(ui->normal->isChecked()){
       EnclDrawSetMonoView(0);
    }
    else if(ui->whiteChartBlackLine->isChecked())
    {
        EnclDrawSetMonoView(-1);
    }else if(ui->blackChartWhiteLine->isChecked())
    {
         EnclDrawSetMonoView(1);
    }

	if(ui->baseDisplay->isChecked())
	{
		EnclDrawSetDisplayCategory(ENCL_BASE);
	}
	else if(ui->standardDisplay->isChecked())
	{
		EnclDrawSetDisplayCategory(ENCL_STANDARD);
	}
	else if (ui->otherDisplay->isChecked())
	{
		EnclDrawSetDisplayCategory(ENCL_OTHER);
	}
	else
	{
		ui->satandardGroupBox->setEnabled(true);
		ui->otherGroupBox->setEnabled(true);

        int groupLayer;
        for (int i =0; i< m_custumCheckBox.size(); ++i)
		{
            groupLayer= i+2;
            true == m_custumCheckBox[i]->isChecked() ?  EnclDrawSetViewGroupLayer(&groupLayer,1,ENCL_VGA_SET):EnclDrawSetViewGroupLayer(&groupLayer,1,ENCL_VGA_CLEAR);
		}
	}
}