/**
 * @file mainwindow.cpp
 * @brief 主窗口类实现
 * @details 顶层窗口初始化、工具栏信号连接、插件 Dock 管理、海图视图创建。
 *          海图功能和 Enclib 初始化委托给内置 ChartPlugin，插件系统通过
 *          PluginManager 统一管理，每个插件包装为 QDockWidget 嵌入布局。
 */

#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "encl.h"

#include "displaycategory.h"
#include "viewinggroupdialog.h"
#include "waterdepthsetting.h"
#include "platformcontrolpanel.h"
#include "eventhistorydialog.h"
#include "plugin/chartplugin/ChartPlugin.h"
#include "graphicsviewwidget.h"

#include <QApplication>
#include "common/logger.h"
#include "common/xmlconfig.h"

/// 便捷日志宏，统一输出到 cpss.log
static inline void logInfo(const QString &msg) {
    Logger::info("%s", msg.toUtf8().constData());
}

/**
 * @brief 构造函数
 * @details 执行 UI 初始化、Dock 布局设置、工具栏信号连接、插件管理器创建，
 *          注册 ChartPlugin（内置），加载外部 DLL 插件，最后初始化状态栏。
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_pluginManager(nullptr),
      m_currentPlugin(nullptr)
{
    ui->setupUi(this);

    setDockNestingEnabled(true);

    // 占位 centralWidget，使 QMainWindow splitter 正常工作；
    // 实际内容全部放在 DockWidget 中
    QWidget *dummyCenter = new QWidget(this);
    dummyCenter->setVisible(false);
    setCentralWidget(dummyCenter);

    // 所有 Dock 区域的 tab 按钮置于顶部，使用扁平文档模式
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDocumentMode(true);

    // 工具栏按钮信号连接
    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::resetView);
    connect(ui->actionDisplaySetting, &QAction::triggered, this, &MainWindow::showDisplaySetting);
    connect(ui->actionEventHistory, &QAction::triggered, this, &MainWindow::showEventHistory);
    connect(ui->actionPlatformControl, &QAction::triggered, this, &MainWindow::showPlatformControl);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::showEventLegend);

    // 插件管理器：先注册内置 ChartPlugin，再扫描 plugins/ 目录加载外部 DLL 插件
    m_pluginManager = new PluginManager(this);
    m_pluginManager->setPluginHost(this);
    connect(m_pluginManager, &PluginManager::pluginLoaded,
            this, &MainWindow::onPluginLoaded);

    m_pluginManager->registerPlugin(new ChartPlugin());

    createChartDock();

    loadPlugins();

    addToolBar(Qt::LeftToolBarArea, ui->toolBar);
    addToolBar(Qt::BottomToolBarArea, ui->pluginToolBar);
    createStatusBar();

    XmlConfig::instance();

    logInfo("init() done, all subsystems ready");
}

/**
 * @brief 析构函数
 * @details 依次删除所有 Dock、工具栏按钮 Action、UI 对象
 */
MainWindow::~MainWindow()
{
    logInfo("MainWindow destructor enter");
    for (auto dock : m_pluginDocks.values()) {
        delete dock;
    }
    for (auto action : m_pluginActions.values()) {
        delete action;
    }
    delete ui;
    logInfo("MainWindow destructor exit");
}

/**
 * @brief 首次显示事件
 * @param event 显示事件
 */
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    logInfo("MainWindow showEvent");
}

/**
 * @brief 关闭事件
 * @param event 关闭事件，直接接受关闭
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    logInfo("MainWindow closeEvent received");
    event->accept();
}

/**
 * @brief 创建海图 Dock 并初始化信号连接
 * @details 从 PluginManager 获取 ChartPlugin，调用其 createWidget() 创建
 *          GraphicsViewWidget，包装为 QDockWidget 停靠到左侧区域，
 *          并将 ChartPlugin::geoPositionUpdated 信号连接到状态栏更新槽。
 */
void MainWindow::createChartDock()
{
    ChartPlugin *cp = chartPlugin();
    if (!cp) {
        logInfo("[Chart] chartPlugin not registered");
        return;
    }

    QWidget *chartWidget = cp->createWidget(this);
    if (!chartWidget) {
        logInfo("[Chart] createWidget returned null");
        return;
    }

    QDockWidget *dock = new QDockWidget(QStringLiteral("海图"), this);
    dock->setObjectName("Dock_chart.plugin");
    dock->setWidget(chartWidget);
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable |
                      QDockWidget::DockWidgetClosable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                          Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dock->setMinimumWidth(400);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // 同步 Dock 映射表（与外部插件保持一致）
    m_pluginDocks["chart.plugin"] = dock;
    m_pluginWidgets["chart.plugin"] = chartWidget;

    // 连接 ChartPlugin 的地理坐标更新信号 → 主窗口状态栏槽
    connect(cp, &ChartPlugin::geoPositionUpdated,
            this, &MainWindow::updateGeoPosition);

    // 工具栏按钮 checked 状态与 Dock 可见性双向同步
    QAction *action = m_pluginActions.value("chart.plugin", nullptr);
    if (action) {
        action->setChecked(true);
        connect(dock, &QDockWidget::visibilityChanged, this,
                [action](bool visible) {
                    if (action && action->isChecked() != visible) {
                        action->blockSignals(true);
                        action->setChecked(visible);
                        action->blockSignals(false);
                    }
                });
    }

    logInfo("[Chart] dock created and shown");
}

/**
 * @brief 便捷方法：从 PluginManager 获取内置 ChartPlugin 实例
 * @return ChartPlugin 指针，未注册则返回 nullptr
 */
ChartPlugin* MainWindow::chartPlugin() const
{
    IPlugin *p = m_pluginManager->getPlugin("chart.plugin");
    return dynamic_cast<ChartPlugin*>(p);
}

/**
 * @brief 加载外部 DLL 插件
 * @details 扫描可执行文件同级 plugins/ 目录，通过 PluginManager::loadPlugins()
 *          动态加载所有 .dll 文件，加载完成后更新 cpss.xml 插件清单。
 */
void MainWindow::loadPlugins()
{
    QString pluginDir = QCoreApplication::applicationDirPath() + "/plugins";
    logInfo("loadPlugins() dir=" + pluginDir);

    QDir d(pluginDir);
    logInfo("dir exists=" + QString::number(d.exists()));
    if (d.exists()) {
        auto files = d.entryInfoList(QStringList() << "*.dll", QDir::Files);
        logInfo("dll count=" + QString::number(files.size()));
        for (auto &f : files) {
            logInfo("  found: " + f.fileName());
        }
    }

    m_pluginManager->loadPlugins(pluginDir);

    // 将已加载插件的元数据写入 cpss.xml 清单
    QList<XmlConfig::PluginInfo> manifest;
    for (IPlugin *plugin : m_pluginManager->getLoadedPlugins()) {
        manifest.append({plugin->pluginId(), plugin->pluginName(), plugin->pluginVersion()});
    }
    XmlConfig::instance().updatePluginManifest(manifest);
}

/**
 * @brief 插件加载完成回调
 * @param plugin 已加载的插件实例
 * @details 由 PluginManager::pluginLoaded 信号触发，为该插件注册工具栏按钮
 */
void MainWindow::onPluginLoaded(IPlugin *plugin)
{
    logInfo("onPluginLoaded: " + plugin->pluginName() + " id=" + plugin->pluginId());
    registerPluginButton(plugin->pluginId(), plugin->pluginName());
}

/**
 * @brief 向宿主注册插件工具栏按钮
 * @param pluginId 插件唯一标识符
 * @param buttonText 按钮显示文本
 * @return 注册成功返回 true
 * @details 创建一个可选中的 QAction 添加到 pluginToolBar，
 *          点击时触发 onPluginActionTriggered 槽切换插件 Dock 显隐。
 */
bool MainWindow::registerPluginButton(const QString &pluginId, const QString &buttonText)
{
    logInfo("registerPluginButton id=" + pluginId + " text=" + buttonText);
    QAction *action = new QAction(buttonText, this);
    action->setObjectName("action_" + pluginId);
    action->setProperty("pluginId", pluginId);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::onPluginActionTriggered);

    ui->pluginToolBar->addAction(action);
    m_pluginActions[pluginId] = action;
    logInfo("pluginToolBar action count=" + QString::number(ui->pluginToolBar->actions().size()));
    return true;
}

/**
 * @brief 注销插件工具栏按钮
 * @param pluginId 插件唯一标识符
 * @details 从 pluginToolBar 移除并删除对应的 QAction
 */
void MainWindow::unregisterPluginButton(const QString &pluginId)
{
    QAction *action = m_pluginActions.take(pluginId);
    if (action) {
        delete action;
    }
}

/**
 * @brief 插件工具栏按钮点击处理
 * @details 根据 cpss.xml 中配置的 closeMode 决定行为：
 *          - closeMode=1（隐藏模式）：已显示则隐藏，已隐藏则显示
 *          - closeMode=2（删除模式）：Dock 不存在则创建，关闭时自动删除
 *          chart.plugin 停靠在左侧，其他插件停靠在右侧。
 */
void MainWindow::onPluginActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    QString pluginId = action->property("pluginId").toString();
    IPlugin *plugin = m_pluginManager->getPlugin(pluginId);
    if (!plugin) {
        action->setChecked(false);
        return;
    }

    // chart.plugin 停靠左侧，其他插件停靠右侧
    Qt::DockWidgetArea dockArea = (pluginId == "chart.plugin")
                                   ? Qt::LeftDockWidgetArea
                                   : Qt::RightDockWidgetArea;

    int closeMode = XmlConfig::instance().pluginCloseMode(pluginId);
    QDockWidget *dock = m_pluginDocks.value(pluginId, nullptr);

    if (closeMode == 2) {
        // 删除模式：按需创建 Dock，关闭时由 Qt::WA_DeleteOnClose 自动删除
        if (!dock) {
            dock = createPluginDock(plugin);
            if (!dock) { action->setChecked(false); return; }
        }
        if (dock->isFloating()) dock->setFloating(false);
        addDockWidget(dockArea, dock);
        dock->show(); dock->raise(); dock->setFocus();
        action->setChecked(true);
        m_currentPlugin = plugin;
    } else {
        // 隐藏模式：切换可见性
        if (!dock) {
            dock = createPluginDock(plugin);
            if (!dock) { action->setChecked(false); return; }
        }
        if (dock->isVisible()) {
            dock->hide();
            action->setChecked(false);
            if (m_currentPlugin == plugin) m_currentPlugin = nullptr;
        } else {
            if (dock->isFloating()) dock->setFloating(false);
            addDockWidget(dockArea, dock);
            dock->show(); dock->raise(); dock->setFocus();
            action->setChecked(true);
            m_currentPlugin = plugin;
        }
    }
}

/**
 * @brief 将插件包装为 QDockWidget
 * @param plugin 要包装的插件实例
 * @return 创建的 Dock 指针，失败返回 nullptr
 * @details 调用 plugin->createWidget() 创建界面，包装为可移动/浮动/关闭的
 *          QDockWidget，并根据 closeMode 设置不同的关闭行为。
 *          删除模式下设置 WA_DeleteOnClose，隐藏模式下同步可见性信号。
 */
QDockWidget* MainWindow::createPluginDock(IPlugin *plugin)
{
    QString pluginId = plugin->pluginId();
    QWidget *widget = plugin->createWidget(this);
    if (!widget) {
        logInfo("createPluginDock: plugin returned null widget for " + pluginId);
        return nullptr;
    }

    m_pluginWidgets[pluginId] = widget;

    QDockWidget *dock = new QDockWidget(plugin->pluginName(), this);
    dock->setObjectName("Dock_" + pluginId);
    dock->setWidget(widget);
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable |
                      QDockWidget::DockWidgetClosable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                          Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dock->setMinimumWidth(320);

    m_pluginDocks[pluginId] = dock;

    QAction *action = m_pluginActions.value(pluginId, nullptr);
    int closeMode = XmlConfig::instance().pluginCloseMode(pluginId);

    if (closeMode == 2) {
        // 删除模式：Dock 关闭时自动析构，destroyed 信号清理映射表
        dock->setAttribute(Qt::WA_DeleteOnClose);
        connect(dock, &QObject::destroyed, this,
                [this, pluginId, action]() {
                    m_pluginDocks.remove(pluginId);
                    m_pluginWidgets.remove(pluginId);
                    if (action) {
                        action->blockSignals(true);
                        action->setChecked(false);
                        action->blockSignals(false);
                    }
                    if (m_currentPlugin && m_currentPlugin->pluginId() == pluginId) {
                        m_currentPlugin = nullptr;
                    }
                });
    } else {
        // 隐藏模式：Dock 关闭时仅隐藏，visibilityChanged 同步按钮状态
        connect(dock, &QDockWidget::visibilityChanged, this,
                [this, pluginId, action](bool visible) {
                    if (action && action->isChecked() != visible) {
                        action->blockSignals(true);
                        action->setChecked(visible);
                        action->blockSignals(false);
                    }
                    if (!visible && m_currentPlugin && m_currentPlugin->pluginId() == pluginId) {
                        m_currentPlugin = nullptr;
                    }
                });
    }

    return dock;
}

/**
 * @brief 显示指定插件的界面（内部方法）
 * @param plugin 目标插件
 * @details 若 Dock 不存在则创建，否则取消浮动并停靠到右侧区域后显示
 */
void MainWindow::showPluginWidget(IPlugin *plugin)
{
    QString pluginId = plugin->pluginId();
    QDockWidget *dock = m_pluginDocks.value(pluginId, nullptr);
    if (!dock) {
        dock = createPluginDock(plugin);
        if (!dock) return;
    }
    if (dock->isFloating()) dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->show(); dock->raise(); dock->setFocus();
    m_currentPlugin = plugin;
}

/**
 * @brief 激活指定部件（IPluginHost 接口实现）
 * @param widget 要激活的插件界面部件
 * @return 找到了对应 Dock 并成功激活返回 true
 */
bool MainWindow::setActiveWidget(QWidget *widget)
{
    for (auto it = m_pluginWidgets.constBegin(); it != m_pluginWidgets.constEnd(); ++it) {
        if (it.value() == widget) {
            QDockWidget *dock = m_pluginDocks.value(it.key(), nullptr);
            if (dock) {
                dock->show(); dock->raise(); dock->setFocus();
                if (m_pluginActions.contains(it.key())) {
                    m_pluginActions[it.key()]->setChecked(true);
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 隐藏所有插件界面，仅显示海图（IPluginHost 接口实现）
 */
void MainWindow::showMapView()
{
    for (auto dock : m_pluginDocks.values()) {
        dock->hide();
    }
    for (auto action : m_pluginActions.values()) {
        action->setChecked(false);
    }
    m_currentPlugin = nullptr;
    // 单独重新显示海图 Dock
    QDockWidget *chartDock = m_pluginDocks.value("chart.plugin", nullptr);
    if (chartDock) chartDock->show();
    QAction *chartAction = m_pluginActions.value("chart.plugin", nullptr);
    if (chartAction) chartAction->setChecked(true);
}

/**
 * @brief 获取海图视图部件（IPluginHost 接口实现）
 * @return ChartPlugin 内部的 GraphicsViewWidget 指针
 */
QWidget* MainWindow::getViewWidget() const
{
    ChartPlugin *cp = chartPlugin();
    return cp ? cp->chartWidget() : nullptr;
}

/**
 * @brief 获取数据管理器单例（IPluginHost 接口实现）
 * @return DataManager 指针
 */
DataManager* MainWindow::getDataManager()
{
    return DataManager::instance();
}

/**
 * @brief 获取应用版本号（IPluginHost 接口实现）
 * @return 版本字符串
 */
QString MainWindow::getAppVersion() const
{
    return QString("1.0.0");
}

/**
 * @brief 获取应用所在目录（IPluginHost 接口实现）
 * @return 可执行文件所在目录的绝对路径
 */
QString MainWindow::getAppPath() const
{
    return QCoreApplication::applicationDirPath();
}

/**
 * @brief 在状态栏显示消息（IPluginHost 接口实现）
 * @param message 消息内容
 */
void MainWindow::showStatusMessage(const QString &message)
{
    ui->statusBar->showMessage(message);
}

/**
 * @brief 弹出通知对话框（IPluginHost 接口实现）
 * @param title 通知标题
 * @param message 通知内容
 */
void MainWindow::showNotification(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

/**
 * @brief 创建状态栏并显示初始就绪消息
 */
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(QStringLiteral("CPSS v1.0 - 就绪"));
}

/** @brief 放大海图，委托给 ChartPlugin */
void MainWindow::zoomIn()
{
    ChartPlugin *cp = chartPlugin();
    if (cp) cp->zoomIn();
}

/** @brief 缩小海图，委托给 ChartPlugin */
void MainWindow::zoomOut()
{
    ChartPlugin *cp = chartPlugin();
    if (cp) cp->zoomOut();
}

/** @brief 重置海图视图，委托给 ChartPlugin */
void MainWindow::resetView()
{
    ChartPlugin *cp = chartPlugin();
    if (cp) cp->resetView();
}

/**
 * @brief 状态栏地理坐标更新槽
 * @param pos 鼠标在海图视图上的屏幕坐标
 * @details 调用 ChartPlugin::screenToGeo 转换为经纬度，
 *          连同当前比例尺一起显示在状态栏右侧。
 */
void MainWindow::updateGeoPosition(QPoint pos)
{
    ChartPlugin *cp = chartPlugin();
    if (!cp || !cp->isEnclibReady()) {
        statusBar()->showMessage(QStringLiteral("海图未初始化"));
        return;
    }
    double lon, lat;
    if (cp->screenToGeo(pos.x(), pos.y(), lon, lat)) {
        double scale = cp->currentScale();
        statusBar()->showMessage(QStringLiteral("经度: %1  纬度: %2  比例尺: %3")
                                 .arg(lon, 0, 'f', 6)
                                 .arg(lat, 0, 'f', 6)
                                 .arg(scale, 0, 'f', 0));
    }
}

/**
 * @brief 显示事件图例对话框
 * @details 以 HTML 表格形式展示阵营颜色、事件图标含义及操作提示
 */
void MainWindow::showEventLegend()
{
    QString legendText =
        "<h2>事件图例</h2><hr/>"
        "<h3>阵营颜色:</h3>"
        "<table border=\"0\" cellpadding=\"5\">"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#00FF00;border:1px solid black;\"></span></td><td> 友方（绿色）</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FF0000;border:1px solid black;\"></span></td><td> 敌方（红色）</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FFFF00;border:1px solid black;\"></span></td><td> 中立（黄色）</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#808080;border:1px solid black;\"></span></td><td> 未知（灰色）</td></tr>"
        "</table><hr/>"
        "<h3>事件图标:</h3>"
        "<table border=\"0\" cellpadding=\"5\">"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FF0000;color:white;text-align:center;font-weight:bold;border:1px solid black;\">!</span></td><td> 告警</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#8B0000;color:white;text-align:center;font-weight:bold;border:1px solid black;\">A</span></td><td> 攻击</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#0000FF;color:white;text-align:center;font-weight:bold;border:1px solid black;\">D</span></td><td> 防御</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#00FFFF;color:black;text-align:center;font-weight:bold;border:1px solid black;\">C</span></td><td> 接触</td></tr>"
        "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FFA500;color:white;text-align:center;font-weight:bold;border:1px solid black;\">X</span></td><td> 损伤</td></tr>"
        "</table><hr/>"
        "<h3>操作说明:</h3>"
        "<ul>"
        "<li>点击目标显示属性框</li>"
        "<li>双击属性框关闭</li>"
        "<li>拖动属性框可移动位置</li>"
        "<li>点击空白区域隐藏所有属性框</li>"
        "</ul>";
    QMessageBox::information(this, QStringLiteral("事件图例"), legendText);
}

/**
 * @brief 显示海图显示设置对话框
 * @details 包含"显示模式"和"显示分组"两个 Tab 页，
 *          所做修改通过 updateChartView 信号实时刷新海图
 */
void MainWindow::showDisplaySetting()
{
    ChartPlugin *cp = chartPlugin();
    GraphicsViewWidget *view = cp ? cp->chartWidget() : nullptr;

    QDialog* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->setWindowTitle(QStringLiteral("海图显示控制"));

    QGridLayout* gridLayout = new QGridLayout(dialog);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    QTabWidget* tabWidget = new QTabWidget();
    gridLayout->addWidget(tabWidget);

    DisplayCategory* pDcd = new DisplayCategory(this);
    if (view) connect(pDcd, SIGNAL(updateChartView()), view, SLOT(updateChart()));
    tabWidget->addTab(pDcd, QString::fromUtf8("显示模式"));

    ViewingGroupDialog* pVgd = new ViewingGroupDialog(this);
    if (view) connect(pVgd, SIGNAL(updateChartView()), view, SLOT(updateChart()));
    tabWidget->addTab(pVgd, QString::fromUtf8("显示分组"));

    dialog->show();
}

/**
 * @brief 显示水深/等高线设置对话框
 */
void MainWindow::showDepthAndContour()
{
    ChartPlugin *cp = chartPlugin();
    GraphicsViewWidget *view = cp ? cp->chartWidget() : nullptr;

    WaterDepthSetting* depthAndContour = new WaterDepthSetting(this);
    if (view) connect(depthAndContour, SIGNAL(updatChartView()), view, SLOT(updateChart()));
    depthAndContour->show();
    depthAndContour->setAttribute(Qt::WA_DeleteOnClose);
}

/**
 * @brief 显示事件历史对话框
 * @details 从 DataManager 获取完整事件历史并填充到对话框
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
 * @details 初始加载海图视图中的平台数据和显示状态，
 *          用户修改后通过 displayStateChanged 信号实时刷新海图
 */
void MainWindow::showPlatformControl()
{
    ChartPlugin *cp = chartPlugin();
    GraphicsViewWidget *view = cp ? cp->chartWidget() : nullptr;

    PlatformControlPanel *panel = new PlatformControlPanel(this);
    panel->setAttribute(Qt::WA_DeleteOnClose);
    if (view) {
        panel->initWithData(view->getDynamicData(), view->getDisplayStates());
        connect(panel, &PlatformControlPanel::displayStateChanged,
                view, &GraphicsViewWidget::updateDisplayState);
    }
    panel->show();
}
