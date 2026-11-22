/**
 * @file OutputWidget.h
 * @brief 输出面板界面：显示 ut::log 输出的各类消息
 * @details 只接收 OutputSink 的信号（已由 Qt 投递到 GUI 线程），所有 GUI 操作在主线程。
 *          支持级别过滤、清空、暂停滚动。
 * @date 2026-11-22
 */

#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>

class OutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OutputWidget(QWidget *parent = nullptr);
    ~OutputWidget();

public slots:
    /** @brief 接收一条消息并追加显示（主线程执行） */
    void onMessageArrived(int level, const QString &text);

private slots:
    void onClearClicked();
    void onFilterToggled();

private:
    /** @brief 根据级别返回显示颜色 */
    static QString colorForLevel(int level);

    /** @brief 级别名称 */
    static QString nameForLevel(int level);

    /** @brief 判断是否显示该级别 */
    bool levelVisible(int level) const;

    QTextEdit    *m_textEdit;
    QCheckBox    *m_chkDebug;
    QCheckBox    *m_chkInfo;
    QCheckBox    *m_chkWarn;
    QCheckBox    *m_chkError;
    QPushButton  *m_clearBtn;
    QToolButton  *m_pauseBtn;

    bool m_paused = false;                            ///< 暂停自动滚动
    QList<QPair<int, QString>> m_buffer;              ///< 缓存 <级别, 已格式化HTML>（切换过滤时重放）
};

#endif // OUTPUTWIDGET_H
