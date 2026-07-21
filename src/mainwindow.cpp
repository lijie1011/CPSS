#define _USE_MATH_DEFINES
#include <cmath>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <cmath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

#include "mainwindow.h"
#include "encl.h"

#include <QApplication>
#include "dynamicdata.h"
#include "datamanager.h"
#include "udpadapter.h"
#include "tcpadapter.h"
#include "common/logger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_viewWidget(nullptr)
{
    init();
    createActions();
    createToolBar();
    createStatusBar();
}

MainWindow::~MainWindow()
{
    EnclDeinitialize();
}

void MainWindow::init()
{
    QString enclibPath = QCoreApplication::applicationDirPath() + QString::fromUtf8(u8"/3dParty/Enclib");

    m_viewWidget = new ViewWidget(this);
    setCentralWidget(m_viewWidget);

    bool ret = EnclInitialize(enclibPath.toStdString().c_str());

    if (!ret) {
        Logger::warn("EnclInitialize failed with path: %s", enclibPath.toStdString().c_str());
        m_viewWidget->setEnclibReady(false);
        QMessageBox msg(this);
        msg.setWindowTitle(QString::fromUtf8(u8"警告"));
        msg.setText(QString::fromUtf8(u8"EnclInitialize() 初始化失败！\n路径: %1\n\n程序将继续运行，但地图可能无法正常显示。").arg(enclibPath));
        msg.setIcon(QMessageBox::Warning);
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    } else {
        Logger::info("EnclInitialize succeeded with path: %s", enclibPath.toStdString().c_str());
        
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
        
        m_viewWidget->setEnclibReady(true);
        m_viewWidget->setChartCenter(121.0, 31.0);
        m_viewWidget->updateChart();
    }

    connect(m_viewWidget, &ViewWidget::updateGeoPosition,
            this, &MainWindow::updateGeoPosition);

    DataManager *dm = DataManager::instance();

    connect(dm, &DataManager::dynamicDataChanged,
            m_viewWidget, &ViewWidget::updateDynamicData);

    UdpAdapter *udpAdapter = new UdpAdapter(this);
    udpAdapter->setLocalPort(12345);
    dm->addAdapter(udpAdapter);

    TcpAdapter *tcpAdapter = new TcpAdapter(this);
    tcpAdapter->setMode(TcpAdapter::ServerMode);
    tcpAdapter->setServerPort(12346);
    dm->addAdapter(tcpAdapter);

    dm->startAllAdapters();

    initTestData();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTestData);
    timer->start(1000);
}

void MainWindow::initTestData()
{
    DataManager *dm = DataManager::instance();
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    QJsonObject ownShipObj;
    ownShipObj["type"] = "ownShip";
    ownShipObj["mmsi"] = "123456789";
    ownShipObj["name"] = "OwnShip";
    ownShipObj["lon"] = 121.5;
    ownShipObj["lat"] = 31.2;
    ownShipObj["heading"] = 45.0;
    ownShipObj["speed"] = 12.5;
    ownShipObj["validDuration"] = 5000;
    dm->onDataReceived(ownShipObj, Protocol_Unknown);

    QJsonObject target1Obj;
    target1Obj["type"] = "aisTarget";
    target1Obj["mmsi"] = "987654321";
    target1Obj["name"] = "MerchantA";
    target1Obj["lon"] = 121.51;
    target1Obj["lat"] = 31.22;
    target1Obj["heading"] = 180.0;
    target1Obj["speed"] = 8.0;
    target1Obj["shipType"] = 5;
    target1Obj["isDanger"] = false;
    target1Obj["validDuration"] = 5000;
    dm->onDataReceived(target1Obj, Protocol_Unknown);

    QJsonObject target2Obj;
    target2Obj["type"] = "aisTarget";
    target2Obj["mmsi"] = "112233445";
    target2Obj["name"] = "FishingB";
    target2Obj["lon"] = 121.48;
    target2Obj["lat"] = 31.18;
    target2Obj["heading"] = 90.0;
    target2Obj["speed"] = 5.0;
    target2Obj["shipType"] = 30;
    target2Obj["isDanger"] = false;
    target2Obj["validDuration"] = 5000;
    dm->onDataReceived(target2Obj, Protocol_Unknown);

    QJsonObject target3Obj;
    target3Obj["type"] = "aisTarget";
    target3Obj["mmsi"] = "556677889";
    target3Obj["name"] = "DangerTarget";
    target3Obj["lon"] = 121.55;
    target3Obj["lat"] = 31.25;
    target3Obj["heading"] = 225.0;
    target3Obj["speed"] = 15.0;
    target3Obj["shipType"] = 50;
    target3Obj["isDanger"] = true;
    target3Obj["validDuration"] = 5000;
    dm->onDataReceived(target3Obj, Protocol_Unknown);

    QJsonObject radarObj;
    radarObj["type"] = "sensor";
    radarObj["id"] = "radar_01";
    radarObj["lon"] = 121.5;
    radarObj["lat"] = 31.2;
    radarObj["radius"] = 10000;
    radarObj["sensorType"] = "radar";
    radarObj["active"] = true;
    radarObj["validDuration"] = 5000;
    dm->onDataReceived(radarObj, Protocol_Unknown);

    QJsonObject sonarObj;
    sonarObj["type"] = "sensor";
    sonarObj["id"] = "sonar_01";
    sonarObj["lon"] = 121.5;
    sonarObj["lat"] = 31.2;
    sonarObj["radius"] = 3000;
    sonarObj["sensorType"] = "sonar";
    sonarObj["active"] = true;
    sonarObj["validDuration"] = 5000;
    dm->onDataReceived(sonarObj, Protocol_Unknown);

    QJsonObject weapon1Obj;
    weapon1Obj["type"] = "weapon";
    weapon1Obj["id"] = "missile_01";
    weapon1Obj["lon"] = 121.5;
    weapon1Obj["lat"] = 31.2;
    weapon1Obj["targetLon"] = 121.55;
    weapon1Obj["targetLat"] = 31.25;
    weapon1Obj["weaponType"] = "missile";
    weapon1Obj["active"] = true;
    weapon1Obj["validDuration"] = 5000;
    dm->onDataReceived(weapon1Obj, Protocol_Unknown);

    QJsonObject marker1Obj;
    marker1Obj["type"] = "marker";
    marker1Obj["id"] = "mark_01";
    marker1Obj["lon"] = 121.52;
    marker1Obj["lat"] = 31.23;
    marker1Obj["label"] = QString::fromLocal8Bit("航道标记");
    marker1Obj["color"] = "#FFD700";
    marker1Obj["validDuration"] = 5000;
    dm->onDataReceived(marker1Obj, Protocol_Unknown);

    QJsonObject marker2Obj;
    marker2Obj["type"] = "marker";
    marker2Obj["id"] = "mark_02";
    marker2Obj["lon"] = 121.47;
    marker2Obj["lat"] = 31.19;
    marker2Obj["label"] = QString::fromLocal8Bit("NoEntry");
    marker2Obj["color"] = "#FF4444";
    marker2Obj["validDuration"] = 5000;
    dm->onDataReceived(marker2Obj, Protocol_Unknown);

    m_testData = dm->getAllData();
}

void MainWindow::updateTestData()
{
    DataManager *dm = DataManager::instance();
    m_testData = dm->getAllData();
    m_testData.timestamp = QDateTime::currentMSecsSinceEpoch();

    double newOwnShipLon = m_testData.ownShip.lon + 0.0001;
    double newOwnShipLat = m_testData.ownShip.lat + 0.00005;

    QJsonObject ownShipObj;
    ownShipObj["type"] = "ownShip";
    ownShipObj["mmsi"] = m_testData.ownShip.mmsi;
    ownShipObj["name"] = m_testData.ownShip.name;
    ownShipObj["lon"] = newOwnShipLon;
    ownShipObj["lat"] = newOwnShipLat;
    ownShipObj["heading"] = fmod(m_testData.ownShip.heading + 0.5, 360.0);
    ownShipObj["speed"] = m_testData.ownShip.speed;
    ownShipObj["validDuration"] = 5000;
    dm->onDataReceived(ownShipObj, Protocol_Unknown);

    QVector<double> newTargetLons(m_testData.aisTargets.size());
    QVector<double> newTargetLats(m_testData.aisTargets.size());

    for (int i = 0; i < m_testData.aisTargets.size(); ++i) {
        AisTarget &target = m_testData.aisTargets[i];
        double angle = (i + 1) * 60.0 * M_PI / 180.0;

        newTargetLons[i] = target.lon + 0.00005 * cos(angle);
        newTargetLats[i] = target.lat + 0.00005 * sin(angle);

        QJsonObject targetObj;
        targetObj["type"] = "aisTarget";
        targetObj["mmsi"] = target.mmsi;
        targetObj["name"] = target.name;
        targetObj["lon"] = newTargetLons[i];
        targetObj["lat"] = newTargetLats[i];
        targetObj["heading"] = fmod(target.heading + 1.0, 360.0);
        targetObj["speed"] = target.speed;
        targetObj["isDanger"] = target.isDanger;
        targetObj["validDuration"] = 5000;
        dm->onDataReceived(targetObj, Protocol_Unknown);
    }

    QJsonObject radarObj;
    radarObj["type"] = "sensor";
    radarObj["id"] = "radar_01";
    radarObj["lon"] = newOwnShipLon;
    radarObj["lat"] = newOwnShipLat;
    radarObj["radius"] = 10000;
    radarObj["sensorType"] = "radar";
    radarObj["active"] = true;
    radarObj["validDuration"] = 5000;
    dm->onDataReceived(radarObj, Protocol_Unknown);

    QJsonObject sonarObj;
    sonarObj["type"] = "sensor";
    sonarObj["id"] = "sonar_01";
    sonarObj["lon"] = newOwnShipLon;
    sonarObj["lat"] = newOwnShipLat;
    sonarObj["radius"] = 3000;
    sonarObj["sensorType"] = "sonar";
    sonarObj["active"] = true;
    sonarObj["validDuration"] = 5000;
    dm->onDataReceived(sonarObj, Protocol_Unknown);

    QJsonObject weaponObj;
    weaponObj["type"] = "weapon";
    weaponObj["id"] = "missile_01";
    weaponObj["lon"] = newOwnShipLon;
    weaponObj["lat"] = newOwnShipLat;
    weaponObj["targetLon"] = newTargetLons[2];
    weaponObj["targetLat"] = newTargetLats[2];
    weaponObj["weaponType"] = "missile";
    weaponObj["active"] = true;
    weaponObj["validDuration"] = 5000;
    dm->onDataReceived(weaponObj, Protocol_Unknown);

    m_testData = dm->getAllData();
}

void MainWindow::createActions()
{
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar(QString::fromUtf8(u8"操作"));

    QAction *zoomInAct = new QAction(QIcon(), QString::fromUtf8(u8"放大"), this);
    zoomInAct->setShortcut(Qt::Key_Plus);
    connect(zoomInAct, &QAction::triggered, this, &MainWindow::zoomIn);
    toolBar->addAction(zoomInAct);

    QAction *zoomOutAct = new QAction(QIcon(), QString::fromUtf8(u8"缩小"), this);
    zoomOutAct->setShortcut(Qt::Key_Minus);
    connect(zoomOutAct, &QAction::triggered, this, &MainWindow::zoomOut);
    toolBar->addAction(zoomOutAct);

    QAction *resetAct = new QAction(QIcon(), QString::fromUtf8(u8"复位"), this);
    resetAct->setShortcut(Qt::Key_R);
    connect(resetAct, &QAction::triggered, this, &MainWindow::resetView);
    toolBar->addAction(resetAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(QString::fromUtf8(u8"CPSS v1.0 - 就绪"));
}

void MainWindow::zoomIn()
{
    if (m_viewWidget) {
        m_viewWidget->zoomIn();
    }
}

void MainWindow::zoomOut()
{
    if (m_viewWidget) {
        m_viewWidget->zoomOut();
    }
}

void MainWindow::resetView()
{
    if (m_viewWidget) {
        m_viewWidget->setChartCenter(121.5, 31.2);
    }
}

void MainWindow::updateGeoPosition(QPoint pos)
{
    double lon, lat;
    EnclTransformScrnToGeo(pos.x(), pos.y(), &lon, &lat);
    statusBar()->showMessage(QString::fromLocal8Bit("经度: %1 纬度: %2").arg(lon, 0, 'f', 6).arg(lat, 0, 'f', 6));
}
