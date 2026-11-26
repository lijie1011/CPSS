/**
 * @file titlebar.h
 * @brief 自绘窗口标题栏（配合 FramelessWindowHint）
 * @details 深色/浅色主题由 QSS (#TitleBar / #TitleMin / #TitleMax / #TitleClose) 控制。
 * @date 2026-11-26
 */

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

class QLabel;
class QToolButton;
class QMouseEvent;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

    void setTitle(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QLabel *m_titleLabel = nullptr;
    QToolButton *m_minBtn = nullptr;
    QToolButton *m_maxBtn = nullptr;
    QToolButton *m_closeBtn = nullptr;
    QPoint m_dragPos;
    bool m_dragging = false;
};

#endif // TITLEBAR_H
