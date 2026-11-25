/**
 * @file PoseAtlasManager.h
 * @brief 姿态图集管理器：懒加载、按类型选模型、模拟横滚
 * @details 3D姿态图集为预渲染 PNG（36航向×7横滚），运行时只做贴图。
 * @date 2026-11-25
 */

#ifndef POSEATLASMANAGER_H
#define POSEATLASMANAGER_H

#include <QObject>
#include <QHash>
#include <QImage>
#include <QString>
#include <QColor>
#include "dynamicdata.h"   // CPSS_DATA_API

class CPSS_DATA_API PoseAtlasManager : public QObject
{
    Q_OBJECT

public:
    static PoseAtlasManager* instance();

    /** @brief 初始化（读取配置、扫描图集目录） */
    void initialize();
    /** @brief 是否启用 3D 姿态图集模式 */
    bool enabled() const { return m_enabled; }
    /** @brief 3D 图标显示尺寸（像素） */
    int iconSize() const { return m_iconSize; }
    /** @brief 类型对应的模型名（包围盒用） */
    QString modelName(const QString &type) const { return modelForType(type); }
    /** @brief 取指定类型/航向/横滚的姿态图（懒加载） */
    QImage poseImage(const QString &type, double heading, double roll, const QColor &campColor);

    /** @brief 根据航向变化率模拟横滚（转弯倾斜） */
    static double simulateRoll(double curHeading, double prevHeading, double dtMs);

private:
    PoseAtlasManager(QObject *parent = nullptr);

    QString modelForType(const QString &type) const;
    bool loadAtlas(const QString &key);
    QString borderForCamp(const QColor &campColor) const;
    QImage colorize(const QImage &gray, const QColor &campColor);

    bool m_enabled = true;
    QString m_atlasDir;
    int m_headingSteps = 36;
    int m_rollSteps = 7;
    int m_tileSize = 128;
    int m_iconSize = 64;    ///< 显示的图标尺寸

    /** type -> 模型文件名（不含扩展名） */
    QHash<QString, QString> m_typeModels;
    /** 已加载图集：model -> QImage */
    QHash<QString, QImage> m_atlases;
    /** 着色缓存：model+camp+heading+roll -> QImage */
    QHash<QString, QImage> m_colored;
    static PoseAtlasManager *s_instance;
};

#endif // POSEATLASMANAGER_H
