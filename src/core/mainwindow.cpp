/**
 * @file mainwindow.cpp
 * @brief 主窗口类实现
 * @details 顶层窗口初始化、工具栏信号连接、插件 Dock 管理、海图视图创建。
 *          海图功能和 Enclib 初始化由独立 DLL 插件（chartplugin）提供，
 *          插件系统通过 PluginManager 统一管理，每个插件包装为 QDockWidget 嵌入布局。
 */

#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QActionGroup>
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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QDialog>
#include <QScrollArea>
#include <QPixmap>
#include <QFrame>
#include <QResizeEvent>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "encl.h"

#include "displaycategory.h"
#include "viewinggroupdialog.h"
#include "waterdepthsetting.h"
#include "platformcontrolpanel.h"
#include "eventhistorydialog.h"
#include "plugin/chartplugin/IChartPlugin.h"

#include <QApplication>
#include "common/logger.h"
#include "common/xmlconfig.h"
#include "common/thememanager.h"
#include "common/IconManager.h"
#include "dynamicdata.h"

/// 便捷日志宏，统一输出到 cpss.log
static inline void logInfo(const QString &msg) {
    Logger::info("%s", msg.toUtf8().constData());
}

/**
 * @brief 构造函数
 * @details 执行 UI 初始化、Dock 布局设置、工具栏信号连接、插件管理器创建，
 *          加载全部 DLL 插件（含海图插件 chartplugin），最后初始化状态栏。
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

    // 工具栏"视图"下拉按钮（放在"帮助"按钮前）：选择白天/黑夜主题
    QToolButton *themeBtn = new QToolButton(ui->toolBar);
    themeBtn->setText(QStringLiteral("视图"));
    themeBtn->setToolTip(QStringLiteral("切换界面主题"));
    themeBtn->setPopupMode(QToolButton::InstantPopup);
    themeBtn->setStyleSheet("QToolButton::menu-indicator { image: none; }");

    QMenu *themeMenu = new QMenu(themeBtn);
    QActionGroup *themeGroup = new QActionGroup(themeMenu);
    themeGroup->setExclusive(true);

    QAction *dayAction = themeMenu->addAction(QStringLiteral("白天"));
    dayAction->setCheckable(true);
    dayAction->setActionGroup(themeGroup);
    QAction *nightAction = themeMenu->addAction(QStringLiteral("黑夜"));
    nightAction->setCheckable(true);
    nightAction->setActionGroup(themeGroup);

    const bool isDark = ThemeManager::instance()->isDark();
    dayAction->setChecked(!isDark);
    nightAction->setChecked(isDark);

    connect(dayAction, &QAction::triggered, this, [this]() {
        if (ThemeManager::instance()->current() != Theme::Day)
            ThemeManager::instance()->apply(Theme::Day);
    });
    connect(nightAction, &QAction::triggered, this, [this]() {
        if (ThemeManager::instance()->current() != Theme::Night)
            ThemeManager::instance()->apply(Theme::Night);
    });

    themeBtn->setMenu(themeMenu);
    ui->toolBar->insertWidget(ui->actionHelp, themeBtn);

    // 插件管理器：创建后扫描 plugins/ 目录加载所有 DLL 插件（含海图插件）
    m_pluginManager = new PluginManager(this);
    m_pluginManager->setPluginHost(this);
    connect(m_pluginManager, &PluginManager::pluginLoaded,
            this, &MainWindow::onPluginLoaded);
    connect(m_pluginManager, &PluginManager::pluginUnloaded,
            this, &MainWindow::onPluginUnloaded);

    // 插件加载（尤其 chartplugin 的 EnclSENCInit 要扫描 2000+ 海图文件，约 25 秒）
    // 延迟到事件循环启动后执行，让主窗口先 show() 出来，避免界面卡死等待。
    QTimer::singleShot(0, this, [this]() {
        logInfo("[Startup] deferred plugin loading begin");
        statusBar()->showMessage(QStringLiteral("正在加载海图和插件，请稍候..."));
        QApplication::processEvents();   // 让状态栏提示立即绘制出来

        loadPlugins();
        // 海图插件为独立 DLL 插件，需在其加载完成后创建海图 Dock
        createChartDock();

        statusBar()->showMessage(QStringLiteral("就绪"), 3000);
        logInfo("[Startup] deferred plugin loading done");
    });

    addToolBar(Qt::TopToolBarArea, ui->toolBar);       // 主工具栏→最上
    addToolBar(Qt::LeftToolBarArea, ui->pluginToolBar);  // 插件栏→最左

    // 工具栏按钮当前无图标，默认 IconOnly 样式会显示为空白按钮；
    // 改为“文字在图标旁”样式，使按钮文字（放大/缩小/插件名等）正常显示。
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->pluginToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    // 为工具栏设置窗口标题：QMainWindow 右键上下文菜单用 windowTitle 作为
    // 各工具栏/停靠栏的显隐勾选项名称；未设置时该菜单项显示为空白（只有勾选框）。
    ui->toolBar->setWindowTitle(QStringLiteral("工具栏"));
    ui->pluginToolBar->setWindowTitle(QStringLiteral("插件栏"));
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
 * @details 从 PluginManager 获取海图 DLL 插件（IChartPlugin），调用其
 *          createWidget() 创建海图视图，包装为 QDockWidget 停靠到左侧区域，
 *          并将海图视图的地理坐标更新信号连接到状态栏更新槽。
 */
void MainWindow::createChartDock()
{
    IChartPlugin *cp = chartPlugin();
    if (!cp) {
        logInfo("[Chart] chartPlugin not loaded");
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

    // 连接海图视图的地理坐标更新信号 → 主窗口状态栏槽
    connect(chartWidget, SIGNAL(updateGeoPosition(QPoint)),
            this, SLOT(updateGeoPosition(QPoint)));

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
 * @brief 便捷方法：从 PluginManager 获取海图 DLL 插件实例
 * @return IChartPlugin 指针，未加载则返回 nullptr
 */
IChartPlugin* MainWindow::chartPlugin() const
{
    IPlugin *p = m_pluginManager->getPlugin("chart.plugin");
    if (!p) return nullptr;
    return qobject_cast<IChartPlugin*>(p->object());
}

/**
 * @brief 加载外部 DLL 插件
 * @details 扫描可执行文件同级 plugins/ 目录，通过 PluginManager::loadPlugins()
 *          动态加载所有 .dll 文件，加载完成后更新 cpss.xml 插件清单。
 */
void MainWindow::loadPlugins()
{
    QString pluginDir = QCoreApplication::applicationDirPath() + "/plugins";
    logInfo("[plug] loadPlugins() dir=" + pluginDir);

    QDir d(pluginDir);
    logInfo("[plug] dir exists=" + QString::number(d.exists()));
    if (d.exists()) {
        auto files = d.entryInfoList(QStringList() << "*.dll", QDir::Files);
        logInfo("[plug] dll count=" + QString::number(files.size()));
        for (auto &f : files) {
            logInfo("[plug]   found: " + f.fileName());
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
    logInfo("[plug] plugin loaded: id=" + plugin->pluginId());
    registerPluginButton(plugin->pluginId(), plugin->pluginName());
}

/**
 * @brief 插件卸载完成回调
 * @param plugin 已卸载的插件实例
 * @details 由 PluginManager::pluginUnloaded 信号触发，注销该插件的工具栏按钮
 */
void MainWindow::onPluginUnloaded(IPlugin *plugin)
{
    logInfo("[plug] plugin unloaded: id=" + plugin->pluginId());
    unregisterPluginButton(plugin->pluginId());
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
    logInfo("[plug] register plugin button id=" + pluginId);
    QAction *action = new QAction(buttonText, this);
    action->setObjectName("action_" + pluginId);
    action->setProperty("pluginId", pluginId);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::onPluginActionTriggered);

    ui->pluginToolBar->addAction(action);
    m_pluginActions[pluginId] = action;
    logInfo("[plug] pluginToolBar action count=" + QString::number(ui->pluginToolBar->actions().size()));
    return true;
}

/**
 * @brief 注销插件工具栏按钮
 * @param pluginId 插件唯一标识符
 * @details 从 pluginToolBar 移除并删除对应的 QAction，并在日志中记录注销信息
 */
void MainWindow::unregisterPluginButton(const QString &pluginId)
{
    logInfo("[plug] unregisterPluginButton id=" + pluginId);
    QAction *action = m_pluginActions.take(pluginId);
    if (action) {
        delete action;
        logInfo("[plug] unregisterPluginButton removed action id=" + pluginId);
    } else {
        logInfo("[plug] unregisterPluginButton no action id=" + pluginId);
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
 * @return IChartPlugin 创建的海图视图控件指针
 */
QWidget* MainWindow::getViewWidget() const
{
    IChartPlugin *cp = chartPlugin();
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

/** @brief 放大海图，委托给海图插件 */
void MainWindow::zoomIn()
{
    IChartPlugin *cp = chartPlugin();
    if (cp) cp->zoomIn();
}

/** @brief 缩小海图，委托给海图插件 */
void MainWindow::zoomOut()
{
    IChartPlugin *cp = chartPlugin();
    if (cp) cp->zoomOut();
}

/** @brief 重置海图视图，委托给海图插件 */
void MainWindow::resetView()
{
    IChartPlugin *cp = chartPlugin();
    if (cp) cp->resetView();
}

/**
 * @brief 状态栏地理坐标更新槽
 * @param pos 鼠标在海图视图上的屏幕坐标
 * @details 调用 IChartPlugin::screenToGeo 转换为经纬度，
 *          连同当前比例尺一起显示在状态栏右侧。
 */
void MainWindow::updateGeoPosition(QPoint pos)
{
    IChartPlugin *cp = chartPlugin();
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
 * @brief 显示图例对话框
 * @details 以自定义对话框展示：阵营颜色（我红敌蓝）、平台类型对应图标（由
 *          IconManager 按阵营色实时着色渲染，与海图显示保持一致）、事件图标
 *          含义及操作提示。平台图例直接复用 iconFileForType()/campColor()
 *          与 IconManager，确保与海图上的绘制完全一致。
 */
void MainWindow::showEventLegend()
{
    // 确保图标管理器已初始化（幂等；若海图插件已初始化则直接返回）
    IconManager::instance().init();

    // 图例对话框（带滚动区域，内容较多时可滚动查看）
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("图例"));
    dialog.resize(420, 640);

    QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);
    dlgLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea(&dialog);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    dlgLayout->addWidget(scroll);

    QWidget *content = new QWidget(scroll);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    // ---- 阵营颜色（我红敌蓝，与 campColor() 保持一致）----
    {
        QLabel *title = new QLabel(QStringLiteral("<h3>阵营颜色</h3>"), content);
        layout->addWidget(title);

        // 阵营顺序：我方/友方/敌方/中立/未知
        const QList<CampType> camps = {Camp_Mine, Camp_Friendly, Camp_Enemy, Camp_Neutral, Camp_Unknown};
        for (CampType camp : camps) {
            QWidget *row = new QWidget(content);
            QHBoxLayout *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 0, 0, 0);
            rl->setSpacing(10);

            // 颜色色块
            QLabel *swatch = new QLabel(row);
            swatch->setFixedSize(22, 22);
            const QColor c = campColor(camp);
            swatch->setStyleSheet(QString("background-color: %1; border: 1px solid #333;").arg(c.name()));
            rl->addWidget(swatch);

            // 阵营名称
            QLabel *name = new QLabel(campName(camp), row);
            name->setStyleSheet("font-size: 14px;");
            rl->addWidget(name);
            rl->addStretch();

            layout->addWidget(row);
        }
    }

    // ---- 平台图例（平台类型 → 图标，实时着色，与海图一致）----
    {
        QLabel *title = new QLabel(QStringLiteral("<h3>平台图例</h3>"), content);
        layout->addWidget(title);

        // 展示的平台类型：type 关键字 + 中文说明
        // （与 iconFileForType() 的映射键保持一致，覆盖测试数据涉及的主要类型）
        const QList<QPair<QString, QString>> entries = {
            {QStringLiteral("carrier"),     QStringLiteral("航空母舰")},
            {QStringLiteral("destroyer"),   QStringLiteral("驱逐舰")},
            {QStringLiteral("frigate"),     QStringLiteral("护卫舰")},
            {QStringLiteral("missileboat"), QStringLiteral("导弹舰")},
            {QStringLiteral("submarine"),   QStringLiteral("潜艇")},
            {QStringLiteral("landingship"), QStringLiteral("登陆舰")},
            {QStringLiteral("service"),     QStringLiteral("勤务船")},
            {QStringLiteral("civilian"),    QStringLiteral("民用船只")},
            {QStringLiteral("awacs"),       QStringLiteral("预警机")},
            {QStringLiteral("bomber"),      QStringLiteral("轰炸机")},
            {QStringLiteral("attacker"),    QStringLiteral("强击机")},
            {QStringLiteral("recon"),       QStringLiteral("侦察机")},
            {QStringLiteral("transport"),   QStringLiteral("运输机")},
            {QStringLiteral("uav"),         QStringLiteral("无人机")},
            {QStringLiteral("fighter"),     QStringLiteral("歼击机")},
            {QStringLiteral("sam"),         QStringLiteral("地空导弹")},
            {QStringLiteral("cruise_sea"),  QStringLiteral("海基巡航导弹")},
            {QStringLiteral("cruise_air"),  QStringLiteral("空基巡航导弹")},
            {QStringLiteral("cruise_land"), QStringLiteral("陆基巡航导弹")},
            {QStringLiteral("ballistic"),   QStringLiteral("弹道导弹")},
        };

        // 平台图例统一以"我方红色"着色示意（阵营色见上方"阵营颜色"说明）
        const QColor legendColor = campColor(Camp_Mine);

        // 以网格布局，每行两列，节省纵向空间
        QWidget *grid = new QWidget(content);
        QGridLayout *gl = new QGridLayout(grid);
        gl->setContentsMargins(0, 0, 0, 0);
        gl->setHorizontalSpacing(16);
        gl->setVerticalSpacing(6);

        int idx = 0;
        for (const auto &e : entries) {
            const int r = idx / 2;
            const int col = (idx % 2) * 2;

            // 图标：通过 IconManager 按类型取模板并着色，与海图绘制完全一致
            QLabel *iconLabel = new QLabel(grid);
            iconLabel->setFixedSize(28, 28);
            iconLabel->setAlignment(Qt::AlignCenter);
            const QString iconFile = iconFileForType(e.first);
            QImage img = IconManager::instance().getIcon(iconFile, legendColor);
            if (!img.isNull()) {
                QPixmap pm = QPixmap::fromImage(
                    img.scaled(26, 26, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                iconLabel->setPixmap(pm);
            } else {
                // 图标缺失时退化显示占位（圆点由海图负责，此处仅文字提示）
                iconLabel->setText(QStringLiteral("—"));
            }
            gl->addWidget(iconLabel, r, col);

            // 说明文字
            QLabel *textLabel = new QLabel(e.second, grid);
            textLabel->setStyleSheet("font-size: 13px;");
            gl->addWidget(textLabel, r, col + 1);

            ++idx;
        }
        layout->addWidget(grid);
    }

    // ---- 事件图标 ----
    {
        QLabel *eventTitle = new QLabel(QStringLiteral("<h3>事件图标</h3>"), content);
        layout->addWidget(eventTitle);

        QLabel *eventBody = new QLabel(content);
        eventBody->setTextFormat(Qt::RichText);
        eventBody->setText(
            "<table border=\"0\" cellpadding=\"4\">"
            "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FF0000;color:white;text-align:center;font-weight:bold;border:1px solid black;\">!</span></td><td> 告警</td></tr>"
            "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#8B0000;color:white;text-align:center;font-weight:bold;border:1px solid black;\">A</span></td><td> 攻击</td></tr>"
            "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#0000FF;color:white;text-align:center;font-weight:bold;border:1px solid black;\">D</span></td><td> 防御</td></tr>"
            "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#00FFFF;color:black;text-align:center;font-weight:bold;border:1px solid black;\">C</span></td><td> 接触</td></tr>"
            "<tr><td><span style=\"display:inline-block;width:20px;height:20px;background-color:#FFA500;color:white;text-align:center;font-weight:bold;border:1px solid black;\">X</span></td><td> 损伤</td></tr>"
            "</table>");
        layout->addWidget(eventBody);
    }

    // ---- 操作说明 ----
    {
        QLabel *opTitle = new QLabel(QStringLiteral("<h3>操作说明</h3>"), content);
        layout->addWidget(opTitle);

        QLabel *opBody = new QLabel(content);
        opBody->setTextFormat(Qt::RichText);
        opBody->setText(
            "<ul>"
            "<li>点击目标显示属性框</li>"
            "<li>双击属性框关闭</li>"
            "<li>拖动属性框可移动位置</li>"
            "<li>点击空白区域隐藏所有属性框</li>"
            "</ul>");
        layout->addWidget(opBody);
    }

    layout->addStretch();
    scroll->setWidget(content);

    // 底部关闭按钮
    QPushButton *closeBtn = new QPushButton(QStringLiteral("关闭"), &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    dlgLayout->addLayout(btnLayout);

    dialog.exec();
}

/**
 * @brief 显示海图显示设置对话框
 * @details 包含"显示模式"和"显示分组"两个 Tab 页，
 *          所做修改通过 updateChartView 信号实时刷新海图
 */
void MainWindow::showDisplaySetting()
{
    IChartPlugin *cp = chartPlugin();

    QDialog* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->setWindowTitle(QStringLiteral("海图显示控制"));

    QGridLayout* gridLayout = new QGridLayout(dialog);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    QTabWidget* tabWidget = new QTabWidget();
    gridLayout->addWidget(tabWidget);

    DisplayCategory* pDcd = new DisplayCategory(this);
    if (cp) connect(pDcd, &DisplayCategory::updateChartView, this, [cp]() { if (cp) cp->updateChartView(); });
    tabWidget->addTab(pDcd, QString::fromUtf8("显示模式"));

    ViewingGroupDialog* pVgd = new ViewingGroupDialog(this);
    if (cp) connect(pVgd, &ViewingGroupDialog::updateChartView, this, [cp]() { if (cp) cp->updateChartView(); });
    tabWidget->addTab(pVgd, QString::fromUtf8("显示分组"));

    dialog->show();
}

/**
 * @brief 显示水深/等高线设置对话框
 */
void MainWindow::showDepthAndContour()
{
    IChartPlugin *cp = chartPlugin();

    WaterDepthSetting* depthAndContour = new WaterDepthSetting(this);
    if (cp) connect(depthAndContour, &WaterDepthSetting::updatChartView, this, [cp]() { if (cp) cp->updateChartView(); });
    depthAndContour->show();
    depthAndContour->setAttribute(Qt::WA_DeleteOnClose);
}

/**
 * @brief 显示事件历史对话框
 * @details 从 DataManager 获取完整事件历史并填充到对话框；
 *          同时连接 DataManager 的事件定向信号 eventAdded/eventRemoved，
 *          使对话框在新增/移除事件时实时刷新（不再依赖已停用的全量
 *          dynamicDataChanged 信号）。对话框设置了 WA_DeleteOnClose，
 *          关闭即销毁，连接以 dialog 为接收者上下文随之自动断开，
 *          避免对话框关闭后信号仍被处理。
 */
void MainWindow::showEventHistory()
{
    EventHistoryDialog *dialog = new EventHistoryDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->updateHistory(DataManager::instance()->getEventHistory());

    // 定向实时刷新：新增事件时用最新历史重建表格
    connect(DataManager::instance(), &DataManager::eventAdded,
            dialog, [dialog](const SpecialEvent &) {
        dialog->updateHistory(DataManager::instance()->getEventHistory());
    });
    // 定向实时刷新：移除事件时同步刷新
    connect(DataManager::instance(), &DataManager::eventRemoved,
            dialog, [dialog](const QString &) {
        dialog->updateHistory(DataManager::instance()->getEventHistory());
    });

    dialog->show();
}

/**
 * @brief 显示平台控制面板
 * @details 初始加载海图视图中的平台数据和显示状态，
 *          用户修改后通过 displayStateChanged 信号实时刷新海图
 */
void MainWindow::showPlatformControl()
{
    IChartPlugin *cp = chartPlugin();

    PlatformControlPanel *panel = new PlatformControlPanel(this);
    panel->setAttribute(Qt::WA_DeleteOnClose);
    if (cp) {
        panel->initWithData(cp->getDynamicData(), cp->getDisplayStates());
        // 通过接口转发显示状态变更，避免跨 DLL 直接调用具体类型
        connect(panel, &PlatformControlPanel::displayStateChanged,
                this, [cp](const DisplayStateMap &stateMap) { if (cp) cp->updateDisplayState(stateMap); });
    }
    panel->show();
}
