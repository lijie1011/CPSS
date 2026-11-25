/**
 * @file attitudesingleview.h
 * @brief 姿态面板中的单个单元：平台下拉 + 实时连续3D姿态投影 + 轨道相机
 * @details 投影算法来自 aircraft_pose_demo.html / gen_models.py，连续角度无档位。
 * @date 2026-11-25
 */

#ifndef ATTITUDESINGLEVIEW_H
#define ATTITUDESINGLEVIEW_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QVector3D>
#include <QHash>
#include <QStringList>

struct PlatformData;
class AttitudeSingleView;

/**
 * @struct ModelMesh
 * @brief 程序化模型（顶点 + 面）
 */
struct ModelMesh {
    QVector<QVector3D> verts;
    struct Face { QList<int> idx; QColor color; };
    QVector<Face> faces;
    void normalize();
};

/** @brief 6 种程序化模型（与 tools/gen_models.py 一致） */
class ModelLibrary
{
public:
    static ModelMesh model(const QString &name);
    static QStringList names();
    static ModelMesh makeFighter();
    static ModelMesh makeDestroyer();
    static ModelMesh makeCarrier();
    static ModelMesh makeDrone();
    static ModelMesh makeRecon();
    static ModelMesh makeMissile();
private:
    static QHash<QString, ModelMesh> s_models;
};

/**
 * @class PlatformCombo
 * @brief 平台选择下拉框：点击时发 clicked 信号（触发刷新列表）
 */
class PlatformCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit PlatformCombo(QWidget *parent = nullptr);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override;
};

/**
 * @class PoseCanvas
 * @brief 姿态画布：实时3D投影 + 轨道相机（鼠标拖拽旋转/滚轮缩放/双击重置）
 */
class PoseCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit PoseCanvas(AttitudeSingleView *owner, QWidget *parent = nullptr);
    /** @brief 重置相机到默认俯视 */
    void resetCamera();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private:
    AttitudeSingleView *m_owner;
    QPoint m_lastPos;
    bool m_dragging = false;
};

class AttitudeSingleView : public QWidget
{
    Q_OBJECT
    friend class PoseCanvas;
public:
    explicit AttitudeSingleView(QWidget *parent = nullptr);

    QString platformId() const;
    void setPlatformId(const QString &id);
    bool isEmpty() const;

    /** @brief 相机水平环绕角（度，0~360） */
    double cameraAzimuth() const { return m_camAz; }
    /** @brief 相机俯仰角（度，-85=仰视, 0=水平, 85=俯视） */
    double cameraElevation() const { return m_camEl; }
    void setCamera(double az, double el, double dist);

public slots:
    void refreshPlatformList();
    void onPlatformUpdated(const PlatformData &p);

signals:
    void platformChanged(const QString &oldId, const QString &newId);

private:
    void drawPose(QPainter &p, const QRect &rect,
                  double heading, double pitch, double roll);
    /** @brief 3D投影：模型姿态 + 轨道相机变换 */
    QPointF project(const QVector3D &v, const QRect &rect,
                    double hd, double pt, double rl, double scale) const;

    PlatformCombo *m_combo = nullptr;
    PoseCanvas *m_canvas = nullptr;
    QLabel *m_label = nullptr;
    QString m_currentId;
    double m_heading = 0.0;
    double m_pitch = 0.0;
    double m_roll = 0.0;
    double m_speed = 0.0;
    QString m_type;
    QString m_name;
    // 轨道相机参数
    double m_camAz = 0.0;     ///< 水平环绕角（度）
    double m_camEl = 75.0;    ///< 相机俯仰（90=正上方俯视, 0=水平侧视）
    double m_camDist = 1.0;   ///< 缩放（0.4~3.0）
    bool m_updatingCombo = false;
};

#endif // ATTITUDESINGLEVIEW_H
