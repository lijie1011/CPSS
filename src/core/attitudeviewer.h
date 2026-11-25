/**
 * @file attitudeviewer.h
 * @brief 姿态面板（QDockWidget）：1~4 个姿态单元 + 相机倾角
 * @details 点击海图平台时优先填入空闲单元；满了不自动替换。
 * @date 2026-11-25
 */

#ifndef ATTITUDEVIEWER_H
#define ATTITUDEVIEWER_H

#include <QDockWidget>

class AttitudeSingleView;
class QGridLayout;
class QSpinBox;
class QSlider;
class QLabel;
struct PlatformData;

class AttitudeViewer : public QDockWidget
{
    Q_OBJECT
public:
    explicit AttitudeViewer(QWidget *parent = nullptr);
    ~AttitudeViewer();

    /** @brief 显示单元数量（1~4） */
    void setViewCount(int n);
    int viewCount() const { return m_count; }

public slots:
    /** @brief 点击海图平台：优先填空，满了忽略 */
    void selectPlatform(const QString &id);
    /** @brief 平台数据更新：转发给对应单元 */
    void onPlatformUpdated(const PlatformData &p);
    /** @brief 平台删除 */
    void onPlatformRemoved(const QString &id);

private slots:
    void onCountChanged(int n);
    void onTiltChanged(int v);
    void onUnitPlatformChanged(const QString &oldId, const QString &newId);

signals:
    /** @brief 面板关闭，MainWindow 负责释放 */
    void closed();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void rebuildLayout();

    QWidget *m_root = nullptr;
    QGridLayout *m_grid = nullptr;
    QVector<AttitudeSingleView*> m_views;
    QSpinBox *m_countSpin = nullptr;
    QSlider *m_tiltSlider = nullptr;
    QLabel *m_tiltLabel = nullptr;
    int m_count = 1;
    double m_tilt = 75.0;
};

#endif // ATTITUDEVIEWER_H
