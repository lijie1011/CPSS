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
    /** @brief 滚动条变化：感知用户是否滚离底部，从而暂停/恢复自动跟随 */
    void onScrollBarValueChanged(int value);

private:
    /** @brief 根据级别返回显示颜色 */
    static QString colorForLevel(int level);

    /** @brief 级别名称 */
    static QString nameForLevel(int level);

    /** @brief 判断是否显示该级别 */
    bool levelVisible(int level) const;

    /** @brief 从 OutputSink 历史重新加载显示（主题切换后刷新颜色用） */
    void reloadHistory();

    QTextEdit    *m_textEdit;
    QCheckBox    *m_chkDebug;
    QCheckBox    *m_chkInfo;
    QCheckBox    *m_chkWarn;
    QCheckBox    *m_chkError;
    QPushButton  *m_clearBtn;

    bool m_autoScroll = true;      ///< 是否自动跟随最新消息（用户滚离底部时暂停）
    bool m_appending = false;      ///< 正在程序化追加/滚动，屏蔽 valueChanged 误判

    QList<QPair<int, QString>> m_buffer;              ///< 缓存 <级别, 已格式化HTML>（切换过滤时重放）
};

#endif // OUTPUTWIDGET_H
