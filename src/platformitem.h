/**
 * @file platformitem.h
 * @brief 平台图元类定义
 * @details 该类定义了在QGraphicsScene中显示的平台图元，负责绘制平台图标、名称标签等。
 * @date 2026-07-28
 */

#ifndef PLATFORMITEM_H
#define PLATFORMITEM_H

#include <QGraphicsItem>
#include <QImage>
#include <QPainterPath>
#include <QColor>

#include "dynamicdata.h"
#include "displaystate.h"

/**
 * @class PlatformItem
 * @brief 平台图元类
 * @details 继承自QGraphicsItem，用于在态势图中显示单个平台
 */
class PlatformItem : public QGraphicsItem
{
public:
    /**
     * @brief 构造函数
     * @param platform 平台数据
     * @param redBoatIcon 红方舰船图标
     * @param redPlaneIcon 红方飞机图标
     * @param purpleBoatIcon 紫方舰船图标
     * @param purplePlaneIcon 紫方飞机图标
     * @param parent 父图元
     */
    PlatformItem(const PlatformData &platform, const QImage &redBoatIcon,
                 const QImage &redPlaneIcon, const QImage &purpleBoatIcon,
                 const QImage &purplePlaneIcon, QGraphicsItem *parent = nullptr);

    /**
     * @brief 更新平台数据
     * @param platform 新的平台数据
     */
    void updateData(const PlatformData &platform);

    /**
     * @brief 更新显示状态
     * @param state 显示状态
     */
    void updateDisplayState(const PlatformDisplayState &state);

    /**
     * @brief 获取边界矩形
     * @return 图元的边界矩形
     */
    QRectF boundingRect() const override;

    /**
     * @brief 绘制图元
     * @param painter 绘制器
     * @param option 样式选项
     * @param widget 父窗口部件
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief 获取平台ID
     * @return 平台ID
     */
    QString id() const { return m_platform.id; }

    /**
     * @brief 检查平台是否过期
     * @return 过期返回true
     */
    bool isExpired() const { return m_platform.isExpired(); }

    /**
     * @brief 获取经度
     * @return 经度值
     */
    double lon() const { return m_platform.lon; }

    /**
     * @brief 获取纬度
     * @return 纬度值
     */
    double lat() const { return m_platform.lat; }

private:
    /**
     * @brief 更新图标
     */
    void updateIcon();

    /**
     * @brief 更新边界矩形
     */
    void updateBoundingRect();

    PlatformData m_platform;            ///< 平台数据
    PlatformDisplayState m_displayState; ///< 显示状态

    QImage m_redBoatIcon;      ///< 红方舰船图标
    QImage m_redPlaneIcon;     ///< 红方飞机图标
    QImage m_purpleBoatIcon;   ///< 紫方舰船图标
    QImage m_purplePlaneIcon;  ///< 紫方飞机图标
    QImage m_currentIcon;      ///< 当前使用的图标
    QImage m_scaledIcon;       ///< 缩放后的图标

    QRectF m_boundingRect; ///< 边界矩形
    QColor m_campColor;    ///< 阵营颜色
    bool m_isMissile;      ///< 是否为导弹
};

#endif