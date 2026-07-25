#include "encl.h"
#include "viewinggroupdialog.h"
#include "ui_viewinggroupdialog.h"

#include <QDomDocument>
#include <QFile>
#include <QTreeWidgetItem>
#include <QCoreApplication>


ViewingGroupDialog::ViewingGroupDialog(QWidget *parent) :
    QDialog(parent),
    firstLoad(true),
    ui(new Ui::ViewingGroupDialog)
{
    ui->setupUi(this);
    init();

    connect(ui->baseTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(treeItemChanged(QTreeWidgetItem*, int)));
    connect(ui->standardTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(treeItemChanged(QTreeWidgetItem*, int)));
    connect(ui->otherTreeWidget,SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(treeItemChanged(QTreeWidgetItem*, int)));

    ui->baseTreeWidget->setEnabled(false);
}

ViewingGroupDialog::~ViewingGroupDialog()
{
    delete ui;
}

void ViewingGroupDialog::init()
{
    QDomDocument domDoc;
    QString strChartInfoXML = QCoreApplication::applicationDirPath() + "/chartinfodisplay.xml";
    QFile fileDisplayCtrl(strChartInfoXML);
    if ( !fileDisplayCtrl.open( QIODevice::ReadOnly ) )
    {
        return;
    }
    if(!domDoc.setContent(&fileDisplayCtrl))
    {
        fileDisplayCtrl.close();
        return;
    }
    firstLoad = false;
    fileDisplayCtrl.close();
    QDomNode nodeViewClass = domDoc.firstChild().nextSibling().firstChild();
    QString strViewClassName;
    QDomNode nodeViewSet;
    QDomNode nodeViewGrp;
    QTreeWidgetItem *itemViewSet,*itemViewGrp;
    while ( !nodeViewClass.isNull())
    {
        strViewClassName = nodeViewClass.nodeName();
        nodeViewSet = nodeViewClass.firstChild();
        while ( !nodeViewSet.isNull())
        {
            if ( strViewClassName == "displaybase")
            {
                itemViewSet = new QTreeWidgetItem(ui->baseTreeWidget);
            }
            else if (strViewClassName == "displaystandard")
            {
                itemViewSet = new QTreeWidgetItem(ui->standardTreeWidget);
            }
            else if (strViewClassName == "displayother")
            {
                itemViewSet = new QTreeWidgetItem(ui->otherTreeWidget);
            }
            itemViewSet->setFlags(Qt::ItemIsUserCheckable);
            itemViewSet->setText(0, nodeViewSet.toElement().attributes().namedItem("desc").nodeValue());
            itemViewSet->setCheckState(0,Qt::Unchecked);

            nodeViewGrp = nodeViewSet.firstChild();
            QString nodeName;
            int checkedNum = 0;
            while ( !nodeViewGrp.isNull())
            {
                itemViewGrp = new QTreeWidgetItem(itemViewSet);
                if (strViewClassName == "displaybase")
                {
                    itemViewGrp->setFlags(Qt::ItemIsUserCheckable);
                }
                else
                {
                    itemViewGrp->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                }
                nodeName = nodeViewGrp.toElement().attributes().namedItem("desc").nodeValue();
                itemViewGrp->setText(0,nodeName );

                int viewingGroupNum = nodeName.left(5).toInt();
                bool checked = EnclDrawGetViewingGroup(viewingGroupNum);
                checked ? itemViewGrp->setCheckState(0,Qt::Checked) : itemViewGrp->setCheckState(0,Qt::Unchecked);
                if(checked == true)
                   checkedNum++;

                nodeViewGrp = nodeViewGrp.nextSibling();
            }
            if(checkedNum == itemViewSet->childCount())
                itemViewSet->setCheckState(0,Qt::Checked);
            else if(checkedNum == 0) {
                itemViewSet->setCheckState(0,Qt::Unchecked);
            }else {
                itemViewSet->setCheckState(0,Qt::PartiallyChecked);
            }

            nodeViewSet = nodeViewSet.nextSibling();

        }
        nodeViewClass = nodeViewClass.nextSibling();
    }

    ui->baseTreeWidget->setHeaderLabel(QString::fromLocal8Bit("基本显示分组"));
    ui->standardTreeWidget->setHeaderLabel(QString::fromLocal8Bit("标准显示分组"));
    ui->otherTreeWidget->setHeaderLabel(QString::fromLocal8Bit("其他显示分组"));
}

void ViewingGroupDialog::updateUI()
{
    if(true == firstLoad)
        init();
    else{

        QTreeWidgetItem* groupTreeItem = NULL;
        int count = ui->baseTreeWidget->topLevelItemCount();
        for (int i = 0; i<count; ++i)
        {
           groupTreeItem= ui->baseTreeWidget->topLevelItem(i);
           updateGroupIem(groupTreeItem);
        }

        count = ui->standardTreeWidget->topLevelItemCount();
        for (int i = 0; i<count; ++i)
        {
           groupTreeItem= ui->standardTreeWidget->topLevelItem(i);
           updateGroupIem(groupTreeItem);
        }

        count = ui->otherTreeWidget->topLevelItemCount();
        for (int i = 0; i<count; ++i)
        {
           groupTreeItem= ui->otherTreeWidget->topLevelItem(i);
           updateGroupIem(groupTreeItem);
        }

    }
}

void ViewingGroupDialog::treeItemChanged(QTreeWidgetItem *item, int column)
{
    EnclDisplayCategory disCategory = EnclDrawGetDisplayCategory();
    if(ENCL_CUSTOM != disCategory )
       EnclDrawSetDisplayCategory(ENCL_CUSTOM);

    Qt::CheckState aimState;
    Qt::CheckState itemState = item->checkState(0);
    if ( Qt::Checked == itemState)
    {
        item->setCheckState(0, Qt::Unchecked);
        aimState =  Qt::Unchecked;
    }
    else if (Qt::PartiallyChecked == itemState)
    {
        item->setCheckState(0, Qt::Checked);
        aimState =  Qt::Checked;
    }
    else if (Qt::Unchecked == itemState)
    {
        item->setCheckState(0, Qt::Checked);
        aimState = Qt::Checked;
    }

    QString itemText = item->text(0);
    int viewingGroupID = itemText.left(5).toInt();
    EnclViewingGroupAction vgAct;
    Qt::Checked == aimState ?  vgAct = EnclViewingGroupAction::ENCL_VGA_SET : vgAct = EnclViewingGroupAction::ENCL_VGA_CLEAR;
    EnclDrawSetViewingGroup(vgAct,&viewingGroupID,1);

    int childCount = item->childCount();
    QTreeWidgetItem* childitem = NULL ;
    QTreeWidgetItem* parentitem = NULL;
    if( NULL == childCount)
    {
       parentitem = item->parent();
       int siblingNum = parentitem->childCount();
       int checkedNum =0;
       for (int i = 0; i< siblingNum ; ++i)
       {
           childitem = parentitem->child(i);
           Qt::CheckState state = childitem->checkState(0);
           if(Qt::Checked == state)
               checkedNum++;
       }

       if(checkedNum == siblingNum)
           parentitem->setCheckState(0,Qt::Checked);
        else if(NULL == checkedNum)
           parentitem->setCheckState(0,Qt::Unchecked);
       else
           parentitem->setCheckState(0,Qt::PartiallyChecked);

    } else if(childCount > 0)
    {
        int childViewingGroupID = 0;
        for (int i =0; i< childCount; ++i)
        {
           childitem = item->child(i);
           childitem->setCheckState(0,aimState);
           childViewingGroupID = childitem->text(0).left(5).toInt();
           EnclDrawSetViewingGroup(vgAct,&childViewingGroupID,1);
        }
    }

    emit updateChartView();
}

void ViewingGroupDialog::updateGroupIem(QTreeWidgetItem * secondaryitem)
{
    if(NULL == secondaryitem)
        return;

    int checkedNum=0;
    for (int j = 0; j<secondaryitem->childCount(); ++j)
    {
        QTreeWidgetItem* item = dynamic_cast<QTreeWidgetItem*>(secondaryitem->child(j));
        QString itemName = item->text(0);
        if(true != itemName.isEmpty())
        {
            int viewingGroupNum = itemName.left(5).toInt();
            if(true == EnclDrawGetViewingGroup(viewingGroupNum) )
            {
                item->setCheckState(0,Qt::Checked);
                checkedNum++;
            }else {
                item->setCheckState(0,Qt::Unchecked);
            }
        }
    }

    if(checkedNum == secondaryitem->childCount())
        secondaryitem->setCheckState(0,Qt::Checked);
    else if(checkedNum == 0) {
        secondaryitem->setCheckState(0,Qt::Unchecked);
    }else {
        secondaryitem->setCheckState(0,Qt::PartiallyChecked);
    }
}