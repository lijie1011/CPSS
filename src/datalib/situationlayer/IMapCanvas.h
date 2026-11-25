/**
 * @file IMapCanvas.h
 * @brief 底图画布坐标接口
 * @details 态势层通过此接口把经纬度转换成场景坐标，不关心底图是 Enclib 还是 WMTS
 * @date 2026-11-25
 */

#ifndef IMAPCANVAS_H
#define IMAPCANVAS_H

#include <QPointF>

/**
 * @class IMapCanvas
 * @brief 地图坐标变换接口（由各底图插件实现）
 */
class IMapCanvas
{
public:
    virtual ~IMapCanvas() = default;

    /** @brief 经纬度 → 场景坐标 */
    virtual QPointF lonLatToScene(double lon, double lat) const = 0;
    /** @brief 场景坐标 → 经纬度 */
    virtual QPointF sceneToLonLat(const QPointF &scene) const = 0;
    /** @brief 当前缩放比例（用于图标大小自适应，1.0=基准） */
    virtual double zoomScale() const = 0;
};

#endif // IMAPCANVAS_H
