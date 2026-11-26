/**
 * @file BoundingBoxItem.cpp
 * @brief 3D 透明包围盒实现
 * @date 2026-11-25
 */

#include "BoundingBoxItem.h"
#include "platformitem.h"
#include "PoseAtlasManager.h"
#include "logger.h"

#include <QPainter>
#include <cmath>

BoundingBoxItem::BoundingBoxItem(PlatformItem *platform, int gridN, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_platform(platform), m_gridN(gridN)
{
    setZValue(1000);   // 确保在所有图元之上，可见
    setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
}

void BoundingBoxItem::modelBounds(QVector3D &mn, QVector3D &mx) const
{
    // 取模型归一化包围盒，然后按"最大边长"生成立方体（方形盒子）
    QVector3D bmn(-0.55,-0.31,-0.06), bmx(0.55,1.0,0.27);   // 默认 fighter
    if (m_platform) {
        const QString model = PoseAtlasManager::instance()->modelName(m_platform->platformData().type);
        if (model == "fighter")          { bmn=QVector3D(-0.5493,-0.3080,-0.0575); bmx=QVector3D(0.5493,0.9998,0.2694); }
        else if (model == "destroyer")   { bmn=QVector3D(-0.1181,-0.9708,-0.2087); bmx=QVector3D(0.1181,0.7347,0.2767); }
        else if (model == "carrier")     { bmn=QVector3D(-0.2961,-0.9370,-0.1851); bmx=QVector3D(0.1422,0.6568,0.2432); }
        else if (model == "drone")       { bmn=QVector3D(-0.9851,-0.5604,-0.0281); bmx=QVector3D(0.9851,0.9877,0.0281); }
        else if (model == "recon")       { bmn=QVector3D(-0.7085,-0.4173,-0.0574); bmx=QVector3D(0.7085,0.9998,0.2048); }
        else if (model == "missile")     { bmn=QVector3D(-0.0551,-0.8102,-0.0551); bmx=QVector3D(0.0551,1.0000,0.0551); }
    }
    // 盒子的长宽高 = 模型的实际包围盒尺寸
    mn = bmn;
    mx = bmx;
}

QPointF BoundingBoxItem::project(const QVector3D &p, double heading, double pitch, double roll) const
{
    // 与 gen_models.py render_pose 完全一致：rotY(roll) → rotX(-pitch) → rotZ(-heading) → rotX(tilt)
    const double PI = 3.14159265358979323846;
    const double rl = roll * PI / 180.0;
    const double pt = -pitch * PI / 180.0;
    const double hd = -heading * PI / 180.0;
    const double tl = 20.0 * PI / 180.0;

    // rotY
    double x = p.x()*std::cos(rl) + p.z()*std::sin(rl);
    double y = p.y();
    double z = -p.x()*std::sin(rl) + p.z()*std::cos(rl);
    // rotX(-pitch)
    double y1 = y*std::cos(pt) - z*std::sin(pt);
    double z1 = y*std::sin(pt) + z*std::cos(pt);
    // rotZ
    double x2 = x*std::cos(hd) - y1*std::sin(hd);
    double y2 = x*std::sin(hd) + y1*std::cos(hd);
    double z2 = z1;
    // rotX(tilt)
    double y3 = y2*std::cos(tl) - z2*std::sin(tl);

    const double iconSize = PoseAtlasManager::instance()->iconSize();
    const double scale = iconSize * 0.42;   // 图集 scale_k
    return QPointF(x2*scale, -y3*scale);    // 屏幕 y 向下，取负
}

QRectF BoundingBoxItem::boundingRect() const
{
    const double iconSize = PoseAtlasManager::instance()->iconSize();
    const double r = iconSize * 1.2;
    return QRectF(-r, -r, 2*r, 2*r);
}

void BoundingBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_platform || !m_platform->usePoseAtlas()) return;

    QVector3D mn, mx;
    modelBounds(mn, mx);
    // 外扩 5%
    const QVector3D ex = (mx-mn)*0.05;
    mn -= ex; mx += ex;

    const double hd = m_platform->platformData().heading;
    const double pt = m_platform->currentPitch();
    const double rl = m_platform->currentRoll();

    painter->save();
    // 青绿色细线（与演示页密度1一致）
    QColor boxColor = PoseAtlasManager::instance()->boundingBoxColor();
    boxColor.setAlpha(230);
    painter->setPen(QPen(boxColor, 1, Qt::SolidLine));

    // 立方体 8 个角点
    const QVector3D c[8] = {
        QVector3D(mn.x(),mn.y(),mn.z()), QVector3D(mx.x(),mn.y(),mn.z()),
        QVector3D(mx.x(),mx.y(),mn.z()), QVector3D(mn.x(),mx.y(),mn.z()),
        QVector3D(mn.x(),mn.y(),mx.z()), QVector3D(mx.x(),mn.y(),mx.z()),
        QVector3D(mx.x(),mx.y(),mx.z()), QVector3D(mn.x(),mx.y(),mx.z())
    };
    // 12 条边（底面4 + 顶面4 + 竖直4）
    static const int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},  // 底面
        {4,5},{5,6},{6,7},{7,4},  // 顶面
        {0,4},{1,5},{2,6},{3,7}   // 竖直
    };
    for (const auto &e : edges) {
        const QPointF pa = project(c[e[0]], hd, pt, rl);
        const QPointF pb = project(c[e[1]], hd, pt, rl);
        painter->drawLine(pa, pb);
    }
    painter->restore();
}
