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

class PlatformControlPanel : public QDialog
{
    Q_OBJECT

public:
    explicit PlatformControlPanel(QWidget *parent = nullptr);
    ~PlatformControlPanel();

    void initWithData(const DynamicObjects &data, const DisplayStateMap &currentStates = DisplayStateMap());

signals:
    void displayStateChanged(const DisplayStateMap &stateMap);

private slots:
    void onApplyClicked();
    void onResetClicked();
    void onCloseClicked();
    void onSearchTextChanged(const QString &text);
    void onTreeItemClicked(QTreeWidgetItem *item, int column);

private:
    void initUI();
    void populateTree(const DynamicObjects &data, const DisplayStateMap &currentStates = DisplayStateMap());
    void addPlatformNode(QTreeWidgetItem *campItem, const PlatformData &platform, const PlatformDisplayState &state = PlatformDisplayState());
    void collectStates(DisplayStateMap &stateMap);

    QString campToString(CampType camp);

    QTreeWidget *m_treeWidget;
    QLineEdit *m_searchEdit;
    QPushButton *m_applyBtn;
    QPushButton *m_resetBtn;
    bool m_updatingCheckState;
    QPushButton *m_closeBtn;

    DynamicObjects m_currentData;
};

#endif