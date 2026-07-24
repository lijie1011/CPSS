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

#include "mainwindow.h"
#include "encl.h"

#include <QApplication>
#include "dynamicdata.h"
#include "datamanager.h"
#include "common/logger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_viewWidget(nullptr),
      m_toolBar(nullptr)
{
    Logger::info("MainWindow constructor: entering");
    m_viewWidget = new ViewWidget(this);
    Logger::info("MainWindow constructor: ViewWidget created");

    createActions();
    Logger::info("MainWindow constructor: createActions done");
    createToolBar();
    Logger::info("MainWindow constructor: createToolBar done");

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    layout->addWidget(m_toolBar);
    layout->addWidget(m_viewWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setCentralWidget(centralWidget);

    Logger::info("MainWindow constructor: setCentralWidget done");
    createStatusBar();
    Logger::info("MainWindow constructor: createStatusBar done");
    Logger::info("MainWindow constructor: exiting");
    init();
}

MainWindow::~MainWindow()
{
    Logger::info("MainWindow destructor called");
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    Logger::info("MainWindow showEvent called");
}

bool MainWindow::checkLicenseExpired(const QString &enclibPath)
{
    QString licPath = enclibPath + "/lic.dat";
    QFile licFile(licPath);
    
    if (!licFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::warn("Cannot open license file: %s", licPath.toStdString().c_str());
        return true;
    }
    
    QTextStream in(&licFile);
    QString content = in.readAll();
    licFile.close();
    
    int pos = content.indexOf("Expires at:");
    if (pos == -1) {
        Logger::warn("Cannot find expiration date in license file");
        return true;
    }
    
    QString dateStr = content.mid(pos + 11).trimmed();
    dateStr = dateStr.left(10);
    
    QDateTime expireDate = QDateTime::fromString(dateStr, "yyyy-MM-dd");
    if (!expireDate.isValid()) {
        Logger::warn("Invalid expiration date format: %s", dateStr.toStdString().c_str());
        return true;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    if (now > expireDate) {
        Logger::warn("License expired at: %s, current time: %s", 
                     dateStr.toStdString().c_str(), 
                     now.toString("yyyy-MM-dd").toStdString().c_str());
        return true;
    }
    
    Logger::info("License valid, expires at: %s", dateStr.toStdString().c_str());
    return false;
}

void MainWindow::init()
{
    Logger::info("MainWindow::init started");
    
    QString enclibPath = QCoreApplication::applicationDirPath() + "/3dParty/Enclib";
    Logger::info("Enclib path: %s", enclibPath.toLocal8Bit().constData());

    bool enclibUsable = false;
    
    Logger::info("Current dir before EnclSENCInit: %s", QDir::currentPath().toLocal8Bit().constData());
    QDir::setCurrent(enclibPath);
    Logger::info("Current dir after setCurrent: %s", QDir::currentPath().toLocal8Bit().constData());

    bool licenseExpired = checkLicenseExpired(enclibPath);
    if (licenseExpired) {
        Logger::warn("License expired, but still trying EnclSENCInit to display chart");
    }

    try {
        bool ret = EnclSENCInit(enclibPath.toLocal8Bit().constData());
        Logger::info("EnclSENCInit returned: %d", ret);

        if (!ret) {
            Logger::warn("EnclSENCInit failed with path: %s", enclibPath.toStdString().c_str());
        } else {
            Logger::info("EnclSENCInit succeeded with path: %s", enclibPath.toStdString().c_str());

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
                Logger::info("EnclDrawChart test succeeded, enclib is usable");
            } else {
                Logger::warn("EnclDrawChart test failed, enclib may have license issues");
            }
        }
    } catch (...) {
        Logger::error("Exception occurred during EnclSENCInit, license may be expired");
    }
    
    m_viewWidget->setEnclibReady(enclibUsable);
    m_viewWidget->updateChart();
    Logger::info("Enclib settings applied, enclibUsable: %d", enclibUsable);

    if (!enclibUsable) {
        Logger::warn("Enclib may have license issues! Path: %s. Map display will show placeholder.", enclibPath.toStdString().c_str());
    }

    connect(m_viewWidget, &ViewWidget::updateGeoPosition,
            this, &MainWindow::updateGeoPosition);
    Logger::info("ViewWidget signals connected");

    DataManager *dataManager = DataManager::instance();
    connect(dataManager, &DataManager::dynamicDataChanged,
            m_viewWidget, &ViewWidget::updateDynamicData);
    dataManager->startTestDataTimer(1000);
    Logger::info("DataManager initialized and test data timer started");
}

void MainWindow::createActions()
{
}

void MainWindow::createToolBar()
{
    m_toolBar = new QToolBar(tr("Operations"), this);

    QAction *zoomInAct = new QAction(QIcon(), tr("Zoom In"), this);
    zoomInAct->setShortcut(Qt::Key_Plus);
    connect(zoomInAct, &QAction::triggered, this, &MainWindow::zoomIn);
    m_toolBar->addAction(zoomInAct);

    QAction *zoomOutAct = new QAction(QIcon(), tr("Zoom Out"), this);
    zoomOutAct->setShortcut(Qt::Key_Minus);
    connect(zoomOutAct, &QAction::triggered, this, &MainWindow::zoomOut);
    m_toolBar->addAction(zoomOutAct);

    QAction *resetAct = new QAction(QIcon(), tr("Reset"), this);
    resetAct->setShortcut(Qt::Key_R);
    connect(resetAct, &QAction::triggered, this, &MainWindow::resetView);
    m_toolBar->addAction(resetAct);

    QAction *legendAct = new QAction(QIcon(), tr("Help"), this);
    legendAct->setShortcut(Qt::Key_F1);
    connect(legendAct, &QAction::triggered, this, &MainWindow::showEventLegend);
    m_toolBar->addAction(legendAct);

    m_toolBar->setOrientation(Qt::Vertical);
    m_toolBar->setIconSize(QSize(24, 24));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("CPSS v1.0 - Ready"));
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
    if (!m_viewWidget || !m_viewWidget->isEnclibReady()) {
        statusBar()->showMessage(tr("Map not initialized"));
        return;
    }
    
    double lon, lat;
    EnclTransformScrnToGeo(pos.x(), pos.y(), &lon, &lat);
    double scale = EnclViewGetScale();
    statusBar()->showMessage(tr("Lon: %1 Lat: %2 Scale: %3").arg(lon, 0, 'f', 6).arg(lat, 0, 'f', 6).arg(scale, 0, 'f', 0));
}

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
