#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QHBoxLayout>
#include <QMap>
#include <QAction>
#include "viewwidget.h"
#include "dynamicdata.h"
#include "eventhistorydialog.h"
#include "plugin/IPlugin.h"
#include "plugin/IPluginHost.h"
#include "plugin/PluginManager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public IPluginHost
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ViewWidget* getViewWidget() const override { return m_viewWidget; }
    DataManager* getDataManager() override;
    QString getAppVersion() const override;
    QString getAppPath() const override;

    void showStatusMessage(const QString &message) override;
    void showNotification(const QString &title, const QString &message) override;

    bool registerPluginButton(const QString &pluginId, const QString &buttonText) override;
    void unregisterPluginButton(const QString &pluginId) override;

    bool setActiveWidget(QWidget *widget) override;
    void showMapView() override;

private:
    void init();
    void createStatusBar();
    void loadPlugins();
    void onPluginLoaded(IPlugin *plugin);
    void showPluginWidget(IPlugin *plugin);
    bool checkLicenseExpired(const QString &enclibPath);
    void showEvent(QShowEvent *event) override;

private slots:
    void zoomIn();
    void zoomOut();
    void resetView();
    void updateGeoPosition(QPoint pos);
    void showEventLegend();
    void onPluginActionTriggered();
    void showDisplaySetting();
    void showDepthAndContour();
    void showEventHistory();
    void showPlatformControl();

private:
    Ui::MainWindow *ui;
    ViewWidget* m_viewWidget;
    PluginManager* m_pluginManager;
    QMap<QString, QAction*> m_pluginActions;
    QMap<QString, QWidget*> m_pluginWidgets;
    QMap<QString, int> m_pluginPageIndices;
    IPlugin* m_currentPlugin;
};

#endif