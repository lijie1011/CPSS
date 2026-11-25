/**
 * @file attitudeviewer.cpp
 * @brief 姿态面板实现：1~4 网格布局 + 倾角 + 优先填空
 * @date 2026-11-25
 */

#include "attitudeviewer.h"
#include "attitudesingleview.h"
#include "common/logger.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QCloseEvent>
#include <QSignalBlocker>

AttitudeViewer::AttitudeViewer(QWidget *parent)
    : QDockWidget(QStringLiteral("姿态显示"), parent)
{
    setObjectName("AttitudeViewer");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_root = new QWidget(this);
    setWidget(m_root);
    QVBoxLayout *rootLay = new QVBoxLayout(m_root);
    rootLay->setContentsMargins(4,4,4,4);
    rootLay->setSpacing(4);

    // 顶部控制条
    QHBoxLayout *ctrl = new QHBoxLayout();
    ctrl->addWidget(new QLabel(QStringLiteral("数量:")));
    m_countSpin = new QSpinBox();
    m_countSpin->setRange(1,4);
    m_countSpin->setValue(1);
    m_countSpin->setFixedWidth(50);
    ctrl->addWidget(m_countSpin);
    ctrl->addWidget(new QLabel(QStringLiteral("  倾角:")));
    m_tiltSlider = new QSlider(Qt::Horizontal);
    m_tiltSlider->setRange(0,90);
    m_tiltSlider->setValue(75);
    m_tiltSlider->setFixedWidth(120);
    m_tiltSlider->setToolTip(QStringLiteral("视角俯仰：90=俯视, 0=侧视（也可在画布上拖拽）"));
    ctrl->addWidget(m_tiltSlider);
    m_tiltLabel = new QLabel(QStringLiteral("75°"));
    ctrl->addWidget(m_tiltLabel);
    ctrl->addStretch();
    rootLay->addLayout(ctrl);

    // 网格容器
    m_grid = new QGridLayout();
    m_grid->setSpacing(4);
    rootLay->addLayout(m_grid, 1);

    rebuildLayout();

    connect(m_countSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AttitudeViewer::onCountChanged);
    connect(m_tiltSlider, &QSlider::valueChanged, this, &AttitudeViewer::onTiltChanged);
}

AttitudeViewer::~AttitudeViewer()
{
    // Qt 父对象自动释放子控件
}

void AttitudeViewer::rebuildLayout()
{
    // 清空布局与旧单元（彻底释放）
    QLayoutItem *item;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            QWidget *w = item->widget();
            w->setParent(nullptr);
            w->deleteLater();   // 避免布局管理中直接 delete 导致悬空
        }
        delete item;
    }
    m_views.clear();

    for (int i=0;i<m_count;i++) {
        AttitudeSingleView *v = new AttitudeSingleView();
        v->setCamera(0.0, m_tilt, 1.0);
        m_views.append(v);
        connect(v, &AttitudeSingleView::platformChanged, this, &AttitudeViewer::onUnitPlatformChanged);
    }

    // 网格排布：1占满 / 2左右 / 3上2下1 / 4田字
    if (m_count==1) {
        m_grid->addWidget(m_views[0], 0,0, 1,2);
    } else if (m_count==2) {
        m_grid->addWidget(m_views[0], 0,0);
        m_grid->addWidget(m_views[1], 0,1);
    } else if (m_count==3) {
        m_grid->addWidget(m_views[0], 0,0);
        m_grid->addWidget(m_views[1], 0,1);
        m_grid->addWidget(m_views[2], 1,0, 1,2);
    } else {
        m_grid->addWidget(m_views[0], 0,0);
        m_grid->addWidget(m_views[1], 0,1);
        m_grid->addWidget(m_views[2], 1,0);
        m_grid->addWidget(m_views[3], 1,1);
    }
    // 先清零，再按数量设置拉伸
    for (int r=0;r<2;r++) m_grid->setRowStretch(r,0);
    for (int c=0;c<2;c++) m_grid->setColumnStretch(c,0);
    if (m_count==1) {
        m_grid->setRowStretch(0,1);
        m_grid->setColumnStretch(0,1);
    } else if (m_count==2) {
        m_grid->setRowStretch(0,1);
        m_grid->setColumnStretch(0,1); m_grid->setColumnStretch(1,1);
    } else {
        for (int r=0;r<2;r++) m_grid->setRowStretch(r,1);
        for (int c=0;c<2;c++) m_grid->setColumnStretch(c,1);
    }
}

void AttitudeViewer::setViewCount(int n)
{
    n = qBound(1,n,4);
    if (n == m_count) return;
    m_count = n;
    if (m_countSpin->value()!=n) { QSignalBlocker b(m_countSpin); m_countSpin->setValue(n); }
    rebuildLayout();
}

void AttitudeViewer::onCountChanged(int n) { setViewCount(n); }

void AttitudeViewer::onTiltChanged(int v)
{
    m_tilt = v;
    m_tiltLabel->setText(QString::number(v)+"°");
    for (auto *view : m_views)
        view->setCamera(view->cameraAzimuth(), v, 1.0);
}

void AttitudeViewer::selectPlatform(const QString &id)
{
    Logger::info("[Attitude] selectPlatform id=%s views=%d", id.toUtf8().constData(), m_views.size());
    if (id.isEmpty()) return;
    // 已显示则忽略
    for (auto *v : m_views) if (v && v->platformId()==id) return;
    // 优先填空；满了不显示
    for (auto *v : m_views) {
        if (v && v->isEmpty()) { v->setPlatformId(id); Logger::info("[Attitude] filled"); return; }
    }
    Logger::info("[Attitude] full, ignored");
}

void AttitudeViewer::onPlatformUpdated(const PlatformData &p)
{
    for (auto *v : m_views) v->onPlatformUpdated(p);
}

void AttitudeViewer::onPlatformRemoved(const QString &id)
{
    for (auto *v : m_views) if (v->platformId()==id) v->setPlatformId(QString());
}

void AttitudeViewer::onUnitPlatformChanged(const QString &, const QString &)
{
    // 下拉选择无需额外处理（单元自更新）
}

void AttitudeViewer::closeEvent(QCloseEvent *event)
{
    emit closed();
    QDockWidget::closeEvent(event);
}
