#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "viewwidget.h"
#include "dynamicdata.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ViewWidget* getViewWidget() const { return m_viewWidget; }

private:
    void init();
    void initTestData();
    void createActions();
    void createToolBar();
    void createStatusBar();

private slots:
    void zoomIn();
    void zoomOut();
    void resetView();
    void updateGeoPosition(QPoint pos);
    void updateTestData();

private:
    ViewWidget* m_viewWidget;
    DynamicObjects m_testData;
};

#endif
