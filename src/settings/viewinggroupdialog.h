#ifndef VIEWINGGROUPDIALOG_H
#define VIEWINGGROUPDIALOG_H

#include <QDialog>
#include <map>

class QTreeWidgetItem;

namespace Ui {
class ViewingGroupDialog;
}

class ViewingGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ViewingGroupDialog(QWidget *parent = NULL);
    ~ViewingGroupDialog();

    void init();

public slots:
    void updateUI();
    void updateGroupIem(QTreeWidgetItem* secondaryitem);
    void treeItemChanged(QTreeWidgetItem* item, int column);

signals:
    void updateChartView();

private:
    Ui::ViewingGroupDialog *ui;

    std::map<int,bool> baseViewGroup;
    std::map<int,bool> standarViewGroup;
    std::map<int,bool> otherViewGroup;

    bool     firstLoad;
};

#endif