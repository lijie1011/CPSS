#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QHBoxLayout>
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
    void createActions();
    void createToolBar();
    void createStatusBar();
    bool checkLicenseExpired(const QString &enclibPath);
    void showEvent(QShowEvent *event) override;

private slots:
    void zoomIn();
    void zoomOut();
    void resetView();
    void updateGeoPosition(QPoint pos);
    void showEventLegend();

private:
    ViewWidget* m_viewWidget;
    QToolBar* m_toolBar;
};

#endif
