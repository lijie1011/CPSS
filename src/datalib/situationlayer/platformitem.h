/**
 * @file platformitem.h
 * @brief 平台图元类定义
 * @details 本类定义了在QGraphicsScene中显示的平台图元，
 *          负责绘制平台图标、名称标签等。
 */

#ifndef PLATFORMITEM_H
#define PLATFORMITEM_H

#include <QGraphicsItem>
#include <QImage>
#include <QPainterPath>
#include <QColor>

#include "dynamicdata.h"
#include "platformdisplaystate.h"

/**
 * @class PlatformItem
 * @brief 平台图元类
 * @details 继承自QGraphicsItem，用于在态势图中显示单个平台
 */
class CPSS_DATA_API PlatformItem : public QGraphicsItem
{
public:
    /**
     * @brief 构造函数
     * @param platform 平台数据
     * @param parent 父图元
     * @details 图标不再由外部预生成传入，而是内部根据平台类型（type）
     *          与阵营（camp）通过 IconManager 按需着色获取，
     *          详见 iconFileForType() 与 campColor() 映射。
     */
    explicit PlatformItem(const PlatformData &platform, QGraphicsItem *parent = nullptr);

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
     * @param widget 父界面
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief 获取平台ID
     * @return 平台ID
     */
    QString id() const { return m_platform.id; }
    /** @brief 获取当前平台数据 */
    const PlatformData &platformData() const { return m_platform; }

    /**
     * @brief 检查平台是否已过期
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
    /** @brief 当前横滚角（3D 包围盒用） */
    double currentRoll() const { return m_currentRoll; }
    /** @brief 当前俯仰角（3D 包围盒用） */
    double currentPitch() const { return m_currentPitch; }
    /** @brief 是否使用 3D 图集模式 */
    bool usePoseAtlas() const { return m_usePoseAtlas; }

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

    QImage m_currentIcon;      ///< 当前使用的图标（按 type+camp 着色后）
    QImage m_scaledIcon;       ///< 缩放后的图标

    QRectF m_boundingRect; ///< 边界矩形
    QColor m_campColor;    ///< 阵营颜色
    bool m_isMissile;      ///< 是否为导弹

    // 3D 姿态图集模式
    QImage m_poseImage;          ///< 当前姿态图（已着色）
    double m_lastHeading = 0.0;  ///< 上帧航向（用于模拟横滚）
    qint64 m_lastUpdateMs = 0;   ///< 上帧时间
    bool m_usePoseAtlas = false; ///< 是否使用姿态图集
    double m_currentRoll = 0.0;  ///< 当前横滚
    double m_currentPitch = 0.0; ///< 当前俯仰
};

#endif
