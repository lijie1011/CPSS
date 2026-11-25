/**
 * @file attitudesingleview.cpp
 * @brief 姿态单元实现：6种程序化模型 + 实时连续3D投影
 * @date 2026-11-25
 */

#include "attitudesingleview.h"
#include "datamanager.h"
#include "dynamicdata.h"
#include "PoseAtlasManager.h"
#include "common/logger.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QEvent>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ===== ModelMesh =====
void ModelMesh::normalize()
{
    if (verts.isEmpty()) return;
    QVector3D ctr;
    for (const auto &v : verts) ctr += v;
    ctr /= verts.size();
    double span = 0.0;
    for (auto &v : verts) { v -= ctr; span = qMax(span, (double)v.length()); }
    if (span > 0) for (auto &v : verts) v /= span;
}

QHash<QString, ModelMesh> ModelLibrary::s_models;

ModelMesh ModelLibrary::makeFighter()
{
    ModelMesh m;
    const double v[][3] = {
        {0,6,0},{-.45,2.2,.25},{.45,2.2,.25},{-.45,2.2,-.25},{.45,2.2,-.25},
        {-0.55,-4,0.3},{0.55,-4,0.3},{-0.55,-4,-0.3},{0.55,-4,-0.3},
        {-4.2,-1.4,0},{-4.2,-2.3,0},{4.2,-1.4,0},{4.2,-2.3,0},
        {-2,-3.2,0},{-2,-3.9,0},{2,-3.2,0},{2,-3.9,0},
        {0,-2.4,0.3},{0,-4,0.3},{0,-3.7,2.2}
    };
    for (const auto &p : v) m.verts.append(QVector3D(p[0],p[1],p[2]));
    struct FD { int i[4]; int n; QColor c; };
    const FD fd[] = {
        {{0,1,2,0},3,QColor(232,232,232)}, {{0,3,4,0},3,QColor(184,184,184)},
        {{1,5,6,2},4,QColor(240,240,240)}, {{3,4,8,7},4,QColor(160,160,160)},
        {{1,3,7,5},4,QColor(200,200,200)}, {{2,6,8,4},4,QColor(192,192,192)},
        {{1,9,10,5},4,QColor(220,220,220)}, {{2,6,12,11},4,QColor(208,208,208)},
        {{5,13,14,0},3,QColor(200,200,200)}, {{6,16,15,0},3,QColor(196,196,196)},
        {{17,19,18,0},3,QColor(144,144,144)}
    };
    for (const auto &f : fd) {
        ModelMesh::Face mf; mf.color = f.c;
        for (int k=0;k<f.n;k++) mf.idx.append(f.i[k]);
        m.faces.append(mf);
    }
    m.normalize();
    return m;
}

ModelMesh ModelLibrary::makeDestroyer()
{
    ModelMesh m;
    // 顶点顺序与 gen_models.py 一致
    const double v[][3] = {
        {0,7,0},{-0.9,5,0.5},{0.9,5,0.5},{-0.9,5,-0.5},{0.9,5,-0.5},
        {-0.9,-6,0.5},{0.9,-6,0.5},{-0.9,-6,-0.5},{0.9,-6,-0.5},
        // 舰桥 box(-.5,.5,2,4,.5,1.8)
        {-0.5,2,0.5},{0.5,2,0.5},{0.5,4,0.5},{-0.5,4,0.5},
        {-0.5,2,1.8},{0.5,2,1.8},{0.5,4,1.8},{-0.5,4,1.8},
        // 后建筑 box(-.3,.3,-2,0,.5,1.2)
        {-0.3,-2,0.5},{0.3,-2,0.5},{0.3,0,0.5},{-0.3,0,0.5},
        {-0.3,-2,1.2},{0.3,-2,1.2},{0.3,0,1.2},{-0.3,0,1.2},
        // 桅杆 box(-.15,.15,3.2,3.6,1.8,3.2)
        {-0.15,3.2,1.8},{0.15,3.2,1.8},{0.15,3.6,1.8},{-0.15,3.6,1.8},
        {-0.15,3.2,3.2},{0.15,3.2,3.2},{0.15,3.6,3.2},{-0.15,3.6,3.2}
    };
    for (const auto &p : v) m.verts.append(QVector3D(p[0],p[1],p[2]));
    auto addF=[&](std::initializer_list<int> idx, QColor c){
        ModelMesh::Face f; f.color=c;
        for (int i:idx) f.idx.append(i);
        m.faces.append(f);
    };
    addF({0,1,2},QColor(216,216,216)); addF({0,3,4},QColor(144,144,144));
    addF({1,2,6,5},QColor(200,200,200)); addF({3,4,8,7},QColor(136,136,136));
    addF({1,3,7,5},QColor(176,176,176)); addF({2,4,8,6},QColor(176,176,176));
    addF({5,6,8,7},QColor(120,120,120));
    auto box=[&](int b,QColor c){
        addF({b,b+1,b+2,b+3},c); addF({b+4,b+5,b+6,b+7},c);
        addF({b,b+1,b+5,b+4},c); addF({b+3,b+2,b+6,b+7},c);
        addF({b,b+3,b+7,b+4},c); addF({b+1,b+2,b+6,b+5},c);
    };
    box(9,QColor(224,224,224)); box(17,QColor(192,192,192)); box(25,QColor(160,160,160));
    m.normalize();
    return m;
}

ModelMesh ModelLibrary::makeCarrier()
{
    ModelMesh m;
    const double v[][3] = {
        {0,8,0},{-2.2,6,0.8},{2.2,6,0.8},{-2.2,-8,0.8},{2.2,-8,0.8},
        // 舰体 box(-2.2,2.2,-8,6,-0.3,0.8)
        {-2.2,-8,-0.3},{2.2,-8,-0.3},{2.2,6,-0.3},{-2.2,6,-0.3},
        {-2.2,-8,0.8},{2.2,-8,0.8},{2.2,6,0.8},{-2.2,6,0.8},
        // 舰岛 box(.8,2,2,4,.8,2.6)
        {0.8,2,0.8},{2,2,0.8},{2,4,0.8},{0.8,4,0.8},
        {0.8,2,2.6},{2,2,2.6},{2,4,2.6},{0.8,4,2.6},
        // 桅杆 box(1.1,1.7,2.4,2.8,2.6,4)
        {1.1,2.4,2.6},{1.7,2.4,2.6},{1.7,2.8,2.6},{1.1,2.8,2.6},
        {1.1,2.4,4},{1.7,2.4,4},{1.7,2.8,4},{1.1,2.8,4}
    };
    for (const auto &p : v) m.verts.append(QVector3D(p[0],p[1],p[2]));
    auto addF=[&](std::initializer_list<int> idx, QColor c){
        ModelMesh::Face f; f.color=c; for (int i:idx) f.idx.append(i); m.faces.append(f);
    };
    addF({0,1,2},QColor(138,138,138));
    addF({1,2,4,3},QColor(154,154,154));
    auto box=[&](int b,QColor c){
        addF({b,b+1,b+2,b+3},c); addF({b+4,b+5,b+6,b+7},c);
        addF({b,b+1,b+5,b+4},c); addF({b+3,b+2,b+6,b+7},c);
        addF({b,b+3,b+7,b+4},c); addF({b+1,b+2,b+6,b+5},c);
    };
    box(5,QColor(96,96,96)); box(13,QColor(176,176,176)); box(21,QColor(144,144,144));
    m.normalize();
    return m;
}

ModelMesh ModelLibrary::makeDrone()
{
    ModelMesh m;
    const double v[][3] = {
        {0,3.5,0},{-0.3,2,0.2},{0.3,2,0.2},{-0.3,-2,0.2},{0.3,-2,0.2},
        {-3.5,0.6,0.05},{3.5,0.6,0.05},{-3.5,-0.6,0.05},{3.5,-0.6,0.05},
        {-1.2,-1.8,0.05},{1.2,-1.8,0.05}
    };
    for (const auto &p : v) m.verts.append(QVector3D(p[0],p[1],p[2]));
    auto addF=[&](std::initializer_list<int> idx, QColor c){
        ModelMesh::Face f; f.color=c; for (int i:idx) f.idx.append(i); m.faces.append(f);
    };
    addF({0,1,2},QColor(224,224,224));
    addF({1,2,4,3},QColor(208,208,208));
    addF({1,5,7,3},QColor(232,232,232)); addF({2,4,8,6},QColor(232,232,232));
    addF({7,8},QColor(192,192,192));
    addF({3,4,10,9},QColor(184,184,184));
    m.normalize();
    return m;
}

ModelMesh ModelLibrary::makeRecon()
{
    ModelMesh m;
    const double v[][3] = {
        {0,6,0},{-0.4,3,0.25},{0.4,3,0.25},{-0.4,-4,0.25},{0.4,-4,0.25},
        {-0.4,3,-0.25},{0.4,3,-0.25},{-0.4,-4,-0.25},{0.4,-4,-0.25},
        {-5,1.2,0},{5,1.2,0},{-5,-0.8,0},{5,-0.8,0},
        {-1.6,-3,0.25},{1.6,-3,0.25},{-1.6,-4,0.25},{1.6,-4,0.25},
        {0,-2.5,0.25},{0,-4,0.25},{0,-3.4,1.6}
    };
    for (const auto &p : v) m.verts.append(QVector3D(p[0],p[1],p[2]));
    const int fi[][4] = {{0,1,2,0},{0,6,5,0},{1,3,4,2},{6,7,8,5},{1,5,7,3},{2,4,8,6},
                         {1,9,10,5},{5,12,11,2},{3,13,14,4},{13,15,16,14},{17,19,18,0}};
    const int fn[] = {3,3,4,4,4,4,4,4,4,4,3};
    const QColor fc[] = {QColor(232,232,232),QColor(184,184,184),QColor(240,240,240),
                         QColor(160,160,160),QColor(200,200,200),QColor(192,192,192),
                         QColor(220,220,220),QColor(196,196,196),QColor(200,200,200),
                         QColor(188,188,188),QColor(144,144,144)};
    for (int i=0;i<11;i++){
        ModelMesh::Face f; f.color=fc[i];
        for (int k=0;k<fn[i];k++) f.idx.append(fi[i][k]);
        m.faces.append(f);
    }
    m.normalize();
    return m;
}

ModelMesh ModelLibrary::makeMissile()
{
    ModelMesh m;
    m.verts.append(QVector3D(0,6.5,0));   // 0 = nose
    for (int i=0;i<8;i++){ double a=i*M_PI/4; m.verts.append(QVector3D(.35*cos(a),4.5,.35*sin(a))); } // 1-8
    for (int i=0;i<8;i++){ double a=i*M_PI/4; m.verts.append(QVector3D(.35*cos(a),-5,.35*sin(a))); } // 9-16
    auto addF=[&](std::initializer_list<int> idx, QColor c){
        ModelMesh::Face f; f.color=c; for (int i:idx) f.idx.append(i); m.faces.append(f);
    };
    for (int i=0;i<8;i++) addF({0, 1+i, 1+(i+1)%8}, i%2?QColor(232,232,232):QColor(216,216,216));
    for (int i=0;i<8;i++) addF({1+i, 1+(i+1)%8, 9+(i+1)%8, 9+i}, i%2?QColor(200,200,200):QColor(184,184,184));
    QList<int> cap; for(int i=0;i<8;i++) cap.append(9+i);
    addF({9,10,11,12,13,14,15,16},QColor(144,144,144));
    m.normalize();
    return m;
}

ModelMesh ModelLibrary::model(const QString &name)
{
    if (s_models.isEmpty()) {
        s_models["fighter"]=makeFighter(); s_models["destroyer"]=makeDestroyer();
        s_models["carrier"]=makeCarrier(); s_models["drone"]=makeDrone();
        s_models["recon"]=makeRecon();     s_models["missile"]=makeMissile();
    }
    // 类型名（aircraft/surface/...）→ 模型名（fighter/destroyer/...）
    const QString modelName = PoseAtlasManager::instance()->modelName(name);
    return s_models.value(modelName, s_models.value("fighter"));  // 返回值拷贝，安全
}

QStringList ModelLibrary::names()
{
    model("fighter"); // 确保初始化
    return s_models.keys();
}

// ===== PlatformCombo =====
PlatformCombo::PlatformCombo(QWidget *parent) : QComboBox(parent) {}

void PlatformCombo::mousePressEvent(QMouseEvent *event)
{
    emit clicked();   // 点击就通知刷新列表（在展开前）
    QComboBox::mousePressEvent(event);
}

// ===== AttitudeSingleView =====
AttitudeSingleView::AttitudeSingleView(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(4,4,4,2);
    lay->setSpacing(2);

    m_combo = new PlatformCombo(this);
    lay->addWidget(m_combo);

    m_canvas = new PoseCanvas(this, this);
    lay->addWidget(m_canvas, 1);   // 中间占满

    m_label = new QLabel(this);
    m_label->setText(QStringLiteral("H:---  P:---  R:---  V:---"));
    m_label->setStyleSheet("color:#858585; font-size:10px;");
    lay->addWidget(m_label);

    setMinimumSize(160,160);
    refreshPlatformList();

    // 点击下拉框时刷新平台列表（新增平台可选）
    connect(m_combo, &PlatformCombo::clicked, this, &AttitudeSingleView::refreshPlatformList);

    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
        if (m_updatingCombo || idx < 0) return;
        const QString newId = m_combo->itemData(idx).toString();
        emit platformChanged(m_currentId, newId);
        m_currentId = newId;
        // 载入数据
        const auto plats = DataManager::instance()->getAllPlatforms();
        for (const auto &p : plats) if (p.id == newId) { onPlatformUpdated(p); break; }
    });
}

QString AttitudeSingleView::platformId() const { return m_currentId; }
bool AttitudeSingleView::isEmpty() const { return m_currentId.isEmpty(); }

void AttitudeSingleView::setCamera(double az, double el, double dist)
{
    m_camAz = std::fmod(az, 360.0);
    if (m_camAz < 0) m_camAz += 360.0;
    m_camEl = qBound(-85.0, el, 90.0);
    m_camDist = qBound(0.4, dist, 3.0);
    if (m_canvas) m_canvas->update();
}

void AttitudeSingleView::setPlatformId(const QString &id)
{
    if (id.isEmpty()) { m_currentId.clear(); m_combo->setCurrentIndex(0); if(m_canvas)m_canvas->update(); return; }

    // 先刷新下拉（面板创建时数据可能还没到），确保 combo 包含该平台
    const int idxBefore = m_combo->findData(id);
    if (idxBefore < 0) refreshPlatformList();

    // 不依赖 combo：直接设当前ID，保证即使列表刷新失败也能显示
    m_currentId = id;
    m_updatingCombo = true;
    const int idx = m_combo->findData(id);
    if (idx >= 0) m_combo->setCurrentIndex(idx);
    m_updatingCombo = false;

    const auto plats = DataManager::instance()->getAllPlatforms();
    for (const auto &p : plats) if (p.id == id) { onPlatformUpdated(p); break; }
    if (m_canvas) m_canvas->update();
}

void AttitudeSingleView::refreshPlatformList()
{
    m_updatingCombo = true;
    const QString old = m_currentId;
    m_combo->clear();
    m_combo->addItem(QStringLiteral("— 未选择 —"), QString());
    const auto plats = DataManager::instance()->getAllPlatforms();
    for (const auto &p : plats) m_combo->addItem(p.name.isEmpty()?p.id:(p.name+" ("+p.id+")"), p.id);
    const int idx = m_combo->findData(old);
    m_combo->setCurrentIndex(idx >= 0 ? idx : 0);
    m_currentId = (idx >= 0) ? old : QString();
    m_updatingCombo = false;
}

void AttitudeSingleView::onPlatformUpdated(const PlatformData &p)
{
    if (p.id != m_currentId || m_currentId.isEmpty()) return;
    m_heading=p.heading; m_pitch=p.pitch; m_roll=p.roll;
    m_speed=p.speed; m_type=p.type; m_name=p.name;
    m_label->setText(QStringLiteral("H:%1°  P:%2°  R:%3°  V:%4节")
                     .arg(m_heading,0,'f',8).arg(m_pitch,0,'f',8)
                     .arg(m_roll,0,'f',8).arg(m_speed,0,'f',0));
    if (m_canvas) m_canvas->update();
}

QPointF AttitudeSingleView::project(const QVector3D &p, const QRect &rect,
                                     double hd, double pt, double rl, double scale) const
{
    const double PI=3.14159265358979323846;
    const double r=rl*PI/180, pi=-pt*PI/180, h=-hd*PI/180;
    // ===== 1. 模型姿态变换（世界坐标 Z=上）=====
    // roll: 绕Y
    double x=p.x()*cos(r)+p.z()*sin(r), y=p.y(), z=-p.x()*sin(r)+p.z()*cos(r);
    // pitch: 绕X
    double y1=y*cos(pi)-z*sin(pi), z1=y*sin(pi)+z*cos(pi);
    // heading: 绕Z
    double xw=x*cos(h)-y1*sin(h), yw=x*sin(h)+y1*cos(h), zw=z1;

    // ===== 2. 轨道相机变换 =====
    // 水平环绕：绕世界Z轴转 -azimuth
    const double az = -m_camAz*PI/180;
    double xc = xw*cos(az) - yw*sin(az);
    double yc = xw*sin(az) + yw*cos(az);
    double zc = zw;
    // 俯仰：绕X轴转 -(90-elevation)
    // el=90(俯视) → 转0，看到XY；el=0(侧视) → 转-90，看到XZ
    const double ex = -(90.0 - m_camEl)*PI/180;
    double ys = yc*cos(ex) - zc*sin(ex);

    // ===== 3. 正交投影（屏幕Y向下，取负）=====
    const double s = scale * m_camDist;
    return QPointF(rect.center().x() + xc*s, rect.center().y() - ys*s);
}

void AttitudeSingleView::drawPose(QPainter &p, const QRect &rect,
                                   double heading, double pitch, double roll)
{
    const ModelMesh mesh = ModelLibrary::model(m_type);
    if (mesh.verts.isEmpty()) return;
    double scale = qMin(rect.width(), rect.height()) * 0.38;
    // 深度排序
    QVector<int> order;
    for (int i=0;i<mesh.faces.size();i++) order.append(i);
    std::sort(order.begin(), order.end(), [&](int a,int b){
        double za=0,zb=0;
        for(int i:mesh.faces[a].idx) za+=mesh.verts[i].z();
        for(int i:mesh.faces[b].idx) zb+=mesh.verts[i].z();
        return za<zb;
    });
    p.setPen(QPen(QColor(64,64,64),1));
    for (int fi : order) {
        const auto &face = mesh.faces[fi];
        QPolygonF poly;
        for (int vi : face.idx) poly << project(mesh.verts[vi],rect,heading,pitch,roll,scale);
        p.setBrush(face.color);
        p.drawPolygon(poly);
    }
}

// ===== PoseCanvas =====
PoseCanvas::PoseCanvas(AttitudeSingleView *owner, QWidget *parent)
    : QWidget(parent), m_owner(owner)
{
    setMinimumSize(120,120);
}

void PoseCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(37,37,38));
    if (!m_owner || m_owner->m_currentId.isEmpty()) {
        p.setPen(QColor(128,128,128)); p.setFont(QFont("Arial",10));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("未选择平台"));
        return;
    }
    const QRect area = rect().adjusted(4,4,-4,-4);
    m_owner->drawPose(p, area, m_owner->m_heading, m_owner->m_pitch, m_owner->m_roll);

    // 轨道相机提示
    p.setPen(QColor(100,100,100)); p.setFont(QFont("Arial",9));
    p.drawText(rect().adjusted(6,6,-6,-6), Qt::AlignTop|Qt::AlignRight,
               QStringLiteral("拖拽旋转 · 滚轮缩放 · 双击重置"));
}

void PoseCanvas::resetCamera()
{
    if (m_owner) m_owner->setCamera(0.0, 75.0, 1.0);
}

void PoseCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void PoseCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && m_owner) {
        const QPoint delta = event->pos() - m_lastPos;
        m_lastPos = event->pos();
        // 水平拖拽 → azimuth（360度环绕）；垂直拖拽 → elevation
        const double newAz = m_owner->cameraAzimuth() - delta.x() * 0.6;
        const double newEl = m_owner->cameraElevation() + delta.y() * 0.6;
        m_owner->setCamera(newAz, newEl, m_owner->m_camDist);
    }
}

void PoseCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void PoseCanvas::wheelEvent(QWheelEvent *event)
{
    if (!m_owner) return;
    const double factor = event->angleDelta().y() > 0 ? 1.12 : 1.0/1.12;
    m_owner->setCamera(m_owner->cameraAzimuth(), m_owner->cameraElevation(),
                       m_owner->m_camDist * factor);
    event->accept();
}

void PoseCanvas::mouseDoubleClickEvent(QMouseEvent *)
{
    resetCamera();
}
