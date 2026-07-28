/**
 * @file mainwindow.cpp
 * @brief 主窗口类实现
 * @details 该类是应用程序的主窗口，负责管理海图显示、工具栏、状态栏、插件系统和各种对话框。
 *          主要功能包括海图初始化、平台控制、事件历史显示、显示设置等。
 * @date 2026-07-28
 */

#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QVBoxLayout>
#include <QPushButton>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "encl.h"

#include "displaycategory.h"
#include "viewinggroupdialog.h"
#include "waterdepthsetting.h"
#include "platformcontrolpanel.h"

#include <QApplication>
#include "dynamicdata.h"
#include "datamanager.h"
#include "common/logger.h"

/**
 * @brief 主窗口构造函数
 * @param parent 父窗口指针
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_viewWidget(nullptr),
      m_pluginManager(nullptr),
      m_currentPlugin(nullptr)
{
    // Logger::info("MainWindow constructor: entering");
    ui->setupUi(this);

    QVBoxLayout *mapLayout = new QVBoxLayout(ui->mapArea);
    mapLayout->setContentsMargins(0, 0, 0, 0);

    m_viewWidget = new GraphicsViewWidget(this);
    mapLayout->addWidget(m_viewWidget);
    // Logger::info("MainWindow constructor: GraphicsViewWidget created");

    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::resetView);
    connect(ui->actionDisplaySetting, &QAction::triggered, this, &MainWindow::showDisplaySetting);
    connect(ui->actionEventHistory, &QAction::triggered, this, &MainWindow::showEventHistory);
    connect(ui->actionPlatformControl, &QAction::triggered, this, &MainWindow::showPlatformControl);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::showEventLegend);
    // Logger::info("MainWindow constructor: actions connected");

    m_pluginManager = new PluginManager(this);
    m_pluginManager->setPluginHost(this);
    connect(m_pluginManager, &PluginManager::pluginLoaded,
            this, &MainWindow::onPluginLoaded);

    addToolBar(Qt::LeftToolBarArea, ui->toolBar);
    addToolBar(Qt::BottomToolBarArea, ui->pluginToolBar);
    createStatusBar();
    // Logger::info("MainWindow constructor: createStatusBar done");
    // Logger::info("MainWindow constructor: exiting");
    init();
}

/**
 * @brief 主窗口析构函数
 */
MainWindow::~MainWindow()
{
    // Logger::info("MainWindow destructor called");

    for (auto action : m_pluginActions.values()) {
        delete action;
    }
    for (auto widget : m_pluginWidgets.values()) {
        delete widget;
    }

    delete ui;
}

/**
 * @brief 显示事件处理
 * @param event 显示事件
 */
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    // Logger::info("MainWindow showEvent called");
}

/**
 * @brief 检查Enclib许可证是否过期
 * @param enclibPath Enclib库路径
 * @return 过期返回true，有效返回false
 */
bool MainWindow::checkLicenseExpired(const QString &enclibPath)
{
    QString licPath = enclibPath + "/lic.dat";
    QFile licFile(licPath);
    
    if (!licFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Logger::warn("Cannot open license file: %s", licPath.toStdString().c_str());
        return true;
    }
    
    QTextStream in(&licFile);
    QString content = in.readAll();
    licFile.close();
    
    int pos = content.indexOf("Expires at:");
    if (pos == -1) {
        // Logger::warn("Cannot find expiration date in license file");
        return true;
    }
    
    QString dateStr = content.mid(pos + 11).trimmed();
    dateStr = dateStr.left(10);
    
    QDateTime expireDate = QDateTime::fromString(dateStr, "yyyy-MM-dd");
    if (!expireDate.isValid()) {
        // Logger::warn("Invalid expiration date format: %s", dateStr.toStdString().c_str());
        return true;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    if (now > expireDate) {
        // Logger::warn("License expired at: %s, current time: %s", 
                     // dateStr.toStdString().c_str(), 
                     // now.toString("yyyy-MM-dd").toStdString().c_str());
        return true;
    }
    
    // Logger::info("License valid, expires at: %s", dateStr.toStdString().c_str());
    return false;
}

/**
 * @brief 初始化主窗口
 * @details 初始化Enclib海图库、视图组件、数据管理器和插件系统
 */
void MainWindow::init()
{
    // Logger::info("MainWindow::init started");
    
    QString enclibPath = QCoreApplication::applicationDirPath() + "/3dParty/Enclib";
    // Logger::info("Enclib path: %s", enclibPath.toLocal8Bit().constData());

    bool enclibUsable = false;
    
    // Logger::info("Current dir before EnclSENCInit: %s", QDir::currentPath().toLocal8Bit().constData());
    QDir::setCurrent(enclibPath);
    // Logger::info("Current dir after setCurrent: %s", QDir::currentPath().toLocal8Bit().constData());

    bool licenseExpired = checkLicenseExpired(enclibPath);
    if (licenseExpired) {
        // Logger::warn("License expired, but still trying EnclSENCInit to display chart");
    }

    try {
        bool ret = EnclSENCInit(enclibPath.toLocal8Bit().constData());
        // Logger::info("EnclSENCInit returned: %d", ret);

        if (!ret) {
            // Logger::warn("EnclSENCInit failed with path: %s", enclibPath.toStdString().c_str());
        } else {
            // Logger::info("EnclSENCInit succeeded with path: %s", enclibPath.toStdString().c_str());

            EnclViewSetScale(4000000);
            EnclViewCenter(121.5, 31.2);

            EnclDrawSetLoadMode(ENCL_LOAD_MODE_AUTO);
            EnclDrawSetShowAccuracy(false);
            EnclDrawSetUseAutoScamin(true);
            EnclDrawSetTextGroupLayer(NULL, 0, ENCL_TGA_SET_ALL);
            EnclDrawSetDisplayNationalLanguage(true);
            EnclDrawSetDisplayChineseLanguage(true);
            EnclDrawSetDisplayCategory(ENCL_BASE);
            EnclDrawSetShowChartBoundary(false);
            EnclDrawSetDisplayCategory(ENCL_CUSTOM);
            EnclDrawSetShowIsolatedDangerObjects(false);

            unsigned char *testPixBuf = EnclDrawChart();
            if (testPixBuf) {
                enclibUsable = true;
                // Logger::info("EnclDrawChart test succeeded, enclib is usable");
            } else {
                // Logger::warn("EnclDrawChart test failed, enclib may have license issues");
            }
        }
    } catch (...) {
        // Logger::error("Exception occurred during EnclSENCInit, license may be expired");
    }
    
    m_viewWidget->setEnclibReady(enclibUsable);
    m_viewWidget->updateChart();
    // Logger::info("Enclib settings applied, enclibUsable: %d", enclibUsable);

    if (!enclibUsable) {
        // Logger::warn("Enclib may have license issues! Path: %s. Map display will show placeholder.", enclibPath.toStdString().c_str());
    }

    connect(m_viewWidget, &GraphicsViewWidget::updateGeoPosition,
            this, &MainWindow::updateGeoPosition);
    // Logger::info("GraphicsViewWidget signals connected");

    DataManager *dataManager = DataManager::instance();
    connect(dataManager, &DataManager::dynamicDataChanged,
            m_viewWidget, &GraphicsViewWidget::updateDynamicData);
    dataManager->startTestDataTimer(100);
    // Logger::info("DataManager initialized and test data timer started");

    loadPlugins();
}

/**
 * @brief 加载插件
 * @details 从应用程序目录下的plugins文件夹加载所有插件
 */
void MainWindow::loadPlugins()
{
    QString pluginDir = QCoreApplication::applicationDirPath() + "/plugins";
    // Logger::info("Loading plugins from: %s", pluginDir.toStdString().c_str());
    m_pluginManager->loadPlugins(pluginDir);
}

/**
 * @brief 插件加载完成处理
 * @param plugin 加载完成的插件
 */
void MainWindow::onPluginLoaded(IPlugin *plugin)
{
    // Logger::info("MainWindow::onPluginLoaded - %s", plugin->pluginName().toStdString().c_str());
    registerPluginButton(plugin->pluginId(), plugin->pluginName());
}

/**
 * @brief 注册插件按钮到工具栏
 * @param pluginId 插件ID
 * @param buttonText 按钮显示文本
 * @return 注册成功返回true
 */
bool MainWindow::registerPluginButton(const QString &pluginId, const QString &buttonText)
{
    QAction *action = new QAction(buttonText, this);
    action->setObjectName("action_" + pluginId);
    action->setProperty("pluginId", pluginId);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::onPluginActionTriggered);

    ui->pluginToolBar->addAction(action);
    m_pluginActions[pluginId] = action;
    return true;
}

/**
 * @brief 注销插件按钮
 * @param pluginId 插件ID
 */
void MainWindow::unregisterPluginButton(const QString &pluginId)
{
    QAction *action = m_pluginActions.take(pluginId);
    if (action) {
        delete action;
    }
}

/**
 * @brief 插件按钮点击处理
 * @details 切换插件界面和海图视图的显示
 */
void MainWindow::onPluginActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    QString pluginId = action->property("pluginId").toString();
    IPlugin *plugin = m_pluginManager->getPlugin(pluginId);
    if (plugin) {
        if (m_currentPlugin == plugin && ui->pluginStack->isVisible()) {
            action->setChecked(false);
            showMapView();
        } else {
            for (auto act : m_pluginActions.values()) {
                act->setChecked(false);
            }
            action->setChecked(true);
            showPluginWidget(plugin);
        }
    }
}

/**
 * @brief 显示插件界面
 * @param plugin 要显示的插件
 */
void MainWindow::showPluginWidget(IPlugin *plugin)
{
    // Logger::info("MainWindow::showPluginWidget - %s", plugin->pluginName().toStdString().c_str());

    QString pluginId = plugin->pluginId();
    QWidget *widget = m_pluginWidgets.value(pluginId, nullptr);

    if (!widget) {
        widget = plugin->createWidget(this);
        m_pluginWidgets[pluginId] = widget;

        QWidget *page = new QWidget(ui->pluginStack);
        int pageIndex = ui->pluginStack->addWidget(page);
        m_pluginPageIndices[pluginId] = pageIndex;

        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->addWidget(widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    ui->pluginStack->setCurrentIndex(m_pluginPageIndices[pluginId]);
    ui->pluginStack->setVisible(true);
    m_currentPlugin = plugin;
}

/**
 * @brief 设置活动插件界面
 * @param widget 插件界面组件
 * @return 设置成功返回true
 */
bool MainWindow::setActiveWidget(QWidget *widget)
{
    for (auto it = m_pluginWidgets.constBegin(); it != m_pluginWidgets.constEnd(); ++it) {
        if (it.value() == widget) {
            int pageIndex = m_pluginPageIndices.value(it.key(), -1);
            if (pageIndex >= 0) {
                ui->pluginStack->setCurrentIndex(pageIndex);
                ui->pluginStack->setVisible(true);
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 显示海图视图
 * @details 隐藏插件界面，显示海图
 */
void MainWindow::showMapView()
{
    for (auto action : m_pluginActions.values()) {
        action->setChecked(false);
    }
    ui->pluginStack->setVisible(false);
    m_currentPlugin = nullptr;
}

/**
 * @brief 获取数据管理器
 * @return DataManager单例指针
 */
DataManager* MainWindow::getDataManager()
{
    return DataManager::instance();
}

/**
 * @brief 获取应用程序版本号
 * @return 版本号字符串
 */
QString MainWindow::getAppVersion() const
{
    return QString("1.0.0");
}

/**
 * @brief 获取应用程序路径
 * @return 应用程序运行目录路径
 */
QString MainWindow::getAppPath() const
{
    return QCoreApplication::applicationDirPath();
}

/**
 * @brief 显示状态栏消息
 * @param message 消息内容
 */
void MainWindow::showStatusMessage(const QString &message)
{
    ui->statusBar->showMessage(message);
}

/**
 * @brief 显示通知对话框
 * @param title 对话框标题
 * @param message 消息内容
 */
void MainWindow::showNotification(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

/**
 * @brief 创建状态栏
 */
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("CPSS v1.0 - Ready"));
}

/**
 * @brief 放大视图
 */
void MainWindow::zoomIn()
{
    if (m_viewWidget) {
        m_viewWidget->zoomIn();
    }
}

/**
 * @brief 缩小视图
 */
void MainWindow::zoomOut()
{
    if (m_viewWidget) {
        m_viewWidget->zoomOut();
    }
}

/**
 * @brief 重置视图到默认位置
 */
void MainWindow::resetView()
{
    if (m_viewWidget) {
        m_viewWidget->setChartCenter(121.5, 31.2);
    }
}

/**
 * @brief 更新地理坐标显示
 * @param pos 鼠标位置
 */
void MainWindow::updateGeoPosition(QPoint pos)
{
    if (!m_viewWidget || !m_viewWidget->isEnclibReady()) {
        statusBar()->showMessage(tr("Map not initialized"));
        return;
    }
    
    double lon, lat;
    EnclTransformScrnToGeo(pos.x(), pos.y(), &lon, &lat);
    double scale = EnclViewGetScale();
    statusBar()->showMessage(tr("Lon: %1 Lat: %2 Scale: %3").arg(lon, 0, 'f', 6).arg(lat, 0, 'f', 6).arg(scale, 0, 'f', 0));
}

/**
 * @brief 显示事件图例对话框
 * @details 显示各类事件和阵营的颜色、图标说明
 */
void MainWindow::showEventLegend()
{
    QString legendText = 
        "<h2>Event Legend</h2>"
        "<hr/>"
        "<h3>Camp Colors:</h3>"
        "<table border=\"0\" cellpadding=\"5\">"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#00FF00;border:1px solid black;\"></span></td><td> Friendly (Green)</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FF0000;border:1px solid black;\"></span></td><td> Enemy (Red)</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FFFF00;border:1px solid black;\"></span></td><td> Neutral (Yellow)</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#808080;border:1px solid black;\"></span></td><td> Unknown (Gray)</td></tr>"
        "</table>"
        "<hr/>"
        "<h3>Event Icons:</h3>"
        "<table border=\"0\" cellpadding=\"5\">"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FF0000;color:white;text-align:center;font-weight:bold;border:1px solid black;\">!</span></td><td> Alert - Warning event</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#8B0000;color:white;text-align:center;font-weight:bold;border:1px solid black;\">A</span></td><td> Attack - Attack event</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#0000FF;color:white;text-align:center;font-weight:bold;border:1px solid black;\">D</span></td><td> Defense - Defense event</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#00FFFF;color:black;text-align:center;font-weight:bold;border:1px solid black;\">C</span></td><td> Contact - Contact event</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FFA500;color:white;text-align:center;font-weight:bold;border:1px solid black;\">X</span></td><td> Damage - Damage event</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#00FF00;color:black;text-align:center;font-weight:bold;border:1px solid black;\">M</span></td><td> Mission Start - Mission started</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#808080;color:white;text-align:center;font-weight:bold;border:1px solid black;\">E</span></td><td> Mission End - Mission ended</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FF00FF;color:white;text-align:center;font-weight:bold;border:1px solid black;\">?</span></td><td> Lost - Target lost</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#90EE90;color:black;text-align:center;font-weight:bold;border:1px solid black;\">R</span></td><td> Repair - Repair event</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FFFF00;color:black;text-align:center;font-weight:bold;border:1px solid black;\">*</span></td><td> Custom - Custom event</td></tr>"
        "</table>"
        "<hr/>"
        "<h3>How to use:</h3>"
        "<ul>"
        "<li>Click on a target to show property box</li>"
        "<li>Double-click on property box to close it</li>"
        "<li>Drag property box to reposition</li>"
        "<li>Click on blank area to hide all property boxes</li>"
        "</ul>";

    QMessageBox::information(this, tr("Event Legend"), legendText);
}

/**
 * @brief 显示海图显示设置对话框
 */
void MainWindow::showDisplaySetting()
{
    QDialog* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->setWindowTitle(QString::fromUtf8("海图显示控制"));

    QGridLayout* gridLayout = new QGridLayout(dialog);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    QTabWidget* tabWidget = new QTabWidget();
    gridLayout->addWidget(tabWidget);

    DisplayCategory* pDcd = new DisplayCategory(this);
    connect(pDcd, SIGNAL(updateChartView()), m_viewWidget, SLOT(updateChart()));
    tabWidget->addTab(pDcd, QString::fromUtf8("显示模式"));

    ViewingGroupDialog* pVgd = new ViewingGroupDialog(this);
    connect(pVgd, SIGNAL(updateChartView()), m_viewWidget, SLOT(updateChart()));
    tabWidget->addTab(pVgd, QString::fromUtf8("显示分组"));

    dialog->show();
}

/**
 * @brief 显示水深和等深线设置对话框
 */
void MainWindow::showDepthAndContour()
{
    WaterDepthSetting* depthAndContour = new WaterDepthSetting(this);
    connect(depthAndContour, SIGNAL(updatChartView()), m_viewWidget, SLOT(updateChart()));
    depthAndContour->show();
    depthAndContour->setAttribute(Qt::WA_DeleteOnClose);
}

/**
 * @brief 显示事件历史对话框
 */
void MainWindow::showEventHistory()
{
    EventHistoryDialog *dialog = new EventHistoryDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->updateHistory(DataManager::instance()->getEventHistory());
    dialog->show();
}

/**
 * @brief 显示平台控制面板
 */
void MainWindow::showPlatformControl()
{
    PlatformControlPanel *panel = new PlatformControlPanel(this);
    panel->setAttribute(Qt::WA_DeleteOnClose);
    panel->initWithData(m_viewWidget->getDynamicData(), m_viewWidget->getDisplayStates());
    connect(panel, &PlatformControlPanel::displayStateChanged,
            m_viewWidget, &GraphicsViewWidget::updateDisplayState);
    panel->show();
}