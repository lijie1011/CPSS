/**
 * @file ChartPlugin.cpp
 * @brief Chart 内置插件实现
 * @details 封装 Enclib 初始化、GraphicsViewWidget 创建、海图控制接口。
 *          本插件不编译为独立 DLL，作为内置插件由 MainWindow 直接注册。
 */

#include "ChartPlugin.h"
#include "graphicsviewwidget.h"
#include "encl.h"
#include "datamanager.h"
#include "common/logger.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>

// 日志辅助函数：通过 Logger 输出信息级别日志
static inline void logInfo(const QString &msg) {
    Logger::info("%s", msg.toUtf8().constData());
}

/**
 * @brief 构造函数
 */
ChartPlugin::ChartPlugin()
    : m_host(nullptr)
    , m_viewWidget(nullptr)
    , m_enclibReady(false)
{
}

/**
 * @brief 析构函数
 * @details m_viewWidget 由其父对象（QDockWidget）接管生命周期，此处不 delete
 */
ChartPlugin::~ChartPlugin()
{
}

/**
 * @brief 获取插件名称
 * @return 插件的显示名称
 */
QString ChartPlugin::pluginName() const { return QStringLiteral("海图"); }

/**
 * @brief 获取插件 ID
 * @return 插件的唯一标识符
 */
QString ChartPlugin::pluginId() const { return QString("chart.plugin"); }

/**
 * @brief 获取插件版本号
 * @return 版本号字符串
 */
QString ChartPlugin::pluginVersion() const { return QString("1.0.0"); }

/**
 * @brief 检查 Enclib 授权文件是否过期
 * @param enclibPath Enclib 库所在目录路径
 * @return 已过期返回 true，未过期返回 false
 */
bool ChartPlugin::checkLicenseExpired(const QString &enclibPath)
{
    QString licPath = enclibPath + "/lic.dat";
    QFile licFile(licPath);
    if (!licFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return true;
    }
    QTextStream in(&licFile);
    QString content = in.readAll();
    licFile.close();

    int pos = content.indexOf("Expires at:");
    if (pos == -1) return true;

    QString dateStr = content.mid(pos + 11).trimmed().left(10);
    QDateTime expireDate = QDateTime::fromString(dateStr, "yyyy-MM-dd");
    if (!expireDate.isValid()) return true;

    QDateTime now = QDateTime::currentDateTime();
    return now > expireDate;
}

/**
 * @brief 初始化 Enclib 海图库
 * @param host 插件宿主接口
 * @return 初始化成功返回 true
 */
bool ChartPlugin::init(IPluginHost *host)
{
    m_host = host;

    QString enclibPath = QCoreApplication::applicationDirPath() + "/3dParty/Enclib";
    logInfo("[ChartPlugin] init enter, path=" + enclibPath);

    QDir::setCurrent(enclibPath);

    checkLicenseExpired(enclibPath);

    bool enclibUsable = false;
    try {
        bool ret = EnclSENCInit(enclibPath.toLocal8Bit().constData());
        logInfo(QString("[ChartPlugin] EnclSENCInit ret=%1").arg(ret));

        if (ret) {
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

            unsigned char *buf = EnclDrawChart();
            if (buf) {
                enclibUsable = true;
            }
        }
    } catch (...) {
        logInfo("[ChartPlugin] enclib init exception");
    }

    m_enclibReady = enclibUsable;
    logInfo(QString("[ChartPlugin] enclibReady=%1").arg(enclibUsable));

    // DataManager 与 GraphicsViewWidget 的连接延后到 createWidget 中，
    // 因为此时 widget 尚未创建，待控件创建完成后再进行连接。
    return true;
}

/**
 * @brief 创建海图视图控件（只创建一次，后续调用返回同一实例）
 * @param parent 父控件
 * @return 海图视图控件指针
 */
QWidget* ChartPlugin::createWidget(QWidget *parent)
{
    if (m_viewWidget) {
        return m_viewWidget;
    }

    m_viewWidget = new GraphicsViewWidget(parent);
    m_viewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_viewWidget->setMinimumWidth(0);
    m_viewWidget->setEnclibReady(m_enclibReady);
    m_viewWidget->updateChart();

    // 将 GraphicsViewWidget 的地理位置更新信号转发给插件自身信号
    connect(m_viewWidget, &GraphicsViewWidget::updateGeoPosition,
            this, &ChartPlugin::geoPositionUpdated);

    // 将 DataManager 的动态数据变更信号连接到 GraphicsViewWidget
    DataManager *dm = DataManager::instance();
    if (dm) {
        connect(dm, &DataManager::dynamicDataChanged,
                m_viewWidget, &GraphicsViewWidget::updateDynamicData);
        dm->startTestDataTimer(100);
    }

    logInfo(QString("[ChartPlugin] createWidget done, view=%1")
            .arg((quintptr)m_viewWidget));
    return m_viewWidget;
}

/**
 * @brief 销毁海图视图控件
 * @param widget 要销毁的控件
 */
void ChartPlugin::destroyWidget(QWidget *widget)
{
    if (widget == m_viewWidget) {
        // 由父对象（QDockWidget）负责析构，此处仅清空指针
        m_viewWidget = nullptr;
    }
}

/**
 * @brief 关闭插件，释放资源
 */
void ChartPlugin::shutdown()
{
    // 无需额外清理；控件由 Dock 接管生命周期
}

/**
 * @brief 放大海图视图
 */
void ChartPlugin::zoomIn()
{
    if (m_viewWidget) m_viewWidget->zoomIn();
}

/**
 * @brief 缩小海图视图
 */
void ChartPlugin::zoomOut()
{
    if (m_viewWidget) m_viewWidget->zoomOut();
}

/**
 * @brief 重置海图视图到默认中心点（121.5, 31.2）
 */
void ChartPlugin::resetView()
{
    if (m_viewWidget) m_viewWidget->setChartCenter(121.5, 31.2);
}

/**
 * @brief 设置海图中心点
 * @param lon 经度
 * @param lat 纬度
 */
void ChartPlugin::setChartCenter(double lon, double lat)
{
    if (m_viewWidget) m_viewWidget->setChartCenter(lon, lat);
}

/**
 * @brief 将屏幕坐标转换为地理坐标
 * @param x 屏幕 X 坐标
 * @param y 屏幕 Y 坐标
 * @param lon 输出经度
 * @param lat 输出纬度
 * @return 转换成功返回 true；Enclib 未就绪时返回 false
 */
bool ChartPlugin::screenToGeo(int x, int y, double &lon, double &lat) const
{
    if (!m_enclibReady) return false;
    EnclTransformScrnToGeo(x, y, &lon, &lat);
    return true;
}

/**
 * @brief 获取当前海图比例尺
 * @return 当前比例尺，Enclib 未就绪时返回 0
 */
double ChartPlugin::currentScale() const
{
    if (!m_enclibReady) return 0;
    return EnclViewGetScale();
}
