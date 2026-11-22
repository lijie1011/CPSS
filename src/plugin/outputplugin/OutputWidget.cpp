/**
 * @file OutputWidget.cpp
 * @brief 输出面板界面实现
 * @date 2026-11-22
 */

#include "OutputWidget.h"
#include "outputsink.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QTextCursor>
#include <QScrollBar>

OutputWidget::OutputWidget(QWidget *parent)
    : QWidget(parent)
{
    // ---- 主体：只读文本显示 ----
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_textEdit->setStyleSheet(
        "QTextEdit { background-color: #FFFFFF; color: #000000; "
        "font-family: Consolas, 'Courier New', monospace; font-size: 12px; }");
    // 不设最大块数限制：Qt 删除旧块会导致内容上移、滚动位置错乱，
    // 破坏"向上翻看历史时暂停跟随"的体验。

    // ---- 顶部：级别过滤 + 操作按钮 ----
    QHBoxLayout *top = new QHBoxLayout();
    top->setSpacing(8);

    m_chkDebug = new QCheckBox(QStringLiteral("Debug"), this);
    m_chkInfo  = new QCheckBox(QStringLiteral("Info"), this);
    m_chkWarn  = new QCheckBox(QStringLiteral("Warn"), this);
    m_chkError = new QCheckBox(QStringLiteral("Error"), this);
    m_chkDebug->setChecked(true);
    m_chkInfo->setChecked(true);
    m_chkWarn->setChecked(true);
    m_chkError->setChecked(true);

    // 用颜色标注复选框文字，直观提示级别颜色
    m_chkDebug->setStyleSheet("QCheckBox { color: #000000; }");
    m_chkInfo->setStyleSheet("QCheckBox { color: #000000; }");
    m_chkWarn->setStyleSheet("QCheckBox { color: #B8860B; }");
    m_chkError->setStyleSheet("QCheckBox { color: #D00000; }");

    connect(m_chkDebug, &QCheckBox::toggled, this, &OutputWidget::onFilterToggled);
    connect(m_chkInfo,  &QCheckBox::toggled, this, &OutputWidget::onFilterToggled);
    connect(m_chkWarn,  &QCheckBox::toggled, this, &OutputWidget::onFilterToggled);
    connect(m_chkError, &QCheckBox::toggled, this, &OutputWidget::onFilterToggled);

    top->addWidget(new QLabel(QStringLiteral("显示:"), this));
    top->addWidget(m_chkDebug);
    top->addWidget(m_chkInfo);
    top->addWidget(m_chkWarn);
    top->addWidget(m_chkError);
    top->addStretch();

    m_clearBtn = new QPushButton(QStringLiteral("清空"), this);
    connect(m_clearBtn, &QPushButton::clicked, this, &OutputWidget::onClearClicked);
    top->addWidget(m_clearBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(4, 4, 4, 4);
    main->setSpacing(4);
    main->addLayout(top);
    main->addWidget(m_textEdit, 1);

    // 监听滚动条：用户滚离底部 → 暂停跟随；滚回底部 → 恢复跟随
    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &OutputWidget::onScrollBarValueChanged);

    // 连接全局输出信号（AutoConnection → 跨线程自动 QueuedConnection 到本线程）
    connect(OutputSink::instance(), &OutputSink::messageArrived,
            this, &OutputWidget::onMessageArrived, Qt::QueuedConnection);

    // 加载已积累的历史消息
    const auto hist = OutputSink::instance()->history();
    for (const auto &item : hist) {
        onMessageArrived((int)item.first, item.second);
    }
}

OutputWidget::~OutputWidget()
{
}

QString OutputWidget::colorForLevel(int level)
{
    switch (static_cast<LogLevel>(level)) {
    case LogLevel::Debug:
    case LogLevel::Info:  return QStringLiteral("#000000");   // 黑
    case LogLevel::Warn:  return QStringLiteral("#B8860B");   // 黄色系（暗金，白底可读）
    case LogLevel::Error: return QStringLiteral("#D00000");   // 红
    default:              return QStringLiteral("#000000");
    }
}

QString OutputWidget::nameForLevel(int level)
{
    switch (static_cast<LogLevel>(level)) {
    case LogLevel::Debug: return QStringLiteral("DEBUG");
    case LogLevel::Info:  return QStringLiteral("INFO");
    case LogLevel::Warn:  return QStringLiteral("WARN");
    case LogLevel::Error: return QStringLiteral("ERROR");
    default:              return QStringLiteral("LOG");
    }
}

bool OutputWidget::levelVisible(int level) const
{
    switch (static_cast<LogLevel>(level)) {
    case LogLevel::Debug: return m_chkDebug->isChecked();
    case LogLevel::Info:  return m_chkInfo->isChecked();
    case LogLevel::Warn:  return m_chkWarn->isChecked();
    case LogLevel::Error: return m_chkError->isChecked();
    default:              return true;
    }
}

void OutputWidget::onMessageArrived(int level, const QString &text)
{
    // 本槽由 QueuedConnection 投递，保证在 GUI 线程执行
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    const QString color = colorForLevel(level);
    const QString name = nameForLevel(level);
    const QString html = QStringLiteral(
        "<span style='color:#888;'>[%1]</span> "
        "<span style='color:%2;'>[%3] %4</span>")
        .arg(ts, color, name, text.toHtmlEscaped());

    // 缓存格式化后的行，供过滤切换时原样重放（保留原时间戳）
    m_buffer.append(qMakePair(level, html));

    if (!levelVisible(level)) return;   // 被过滤的级别暂存，切换过滤时可重放

    // 程序化追加期间屏蔽 valueChanged，避免把程序滚动误判成用户操作
    // QTextEdit::append() 自身会强制滚到底部，所以先看是否在跟随：
    //   - 跟随中：追加后停在最底部看最新
    //   - 未跟随（用户翻历史）：记录位置，追加后恢复，视图不动
    QScrollBar *bar = m_textEdit->verticalScrollBar();
    int savedPos = m_autoScroll ? -1 : bar->value();

    m_appending = true;
    m_textEdit->append(html);
    if (m_autoScroll) {
        m_textEdit->moveCursor(QTextCursor::End);
        bar->setValue(bar->maximum());
    } else {
        bar->setValue(savedPos);   // 恢复用户正在看的位置，不被新消息打断
    }
    m_appending = false;
}

void OutputWidget::onScrollBarValueChanged(int value)
{
    // 程序自己在滚动（append / setValue），不是用户操作，忽略
    if (m_appending) return;

    QScrollBar *bar = m_textEdit->verticalScrollBar();
    // 距底部 20 像素内视为"在底部"，给用户一点容差
    const int threshold = 20;
    bool atBottom = (value >= bar->maximum() - threshold);
    m_autoScroll = atBottom;
}

void OutputWidget::onClearClicked()
{
    m_textEdit->clear();
    m_buffer.clear();
    OutputSink::instance()->clearHistory();
}

void OutputWidget::onFilterToggled()
{
    // 用缓存的 HTML 原样重绘（保留原时间戳与颜色），仅应用新的过滤条件
    m_appending = true;
    m_textEdit->clear();
    for (const auto &item : m_buffer) {
        if (levelVisible(item.first)) {
            m_textEdit->append(item.second);
        }
    }
    if (m_autoScroll) {
        m_textEdit->moveCursor(QTextCursor::End);
        m_textEdit->verticalScrollBar()->setValue(m_textEdit->verticalScrollBar()->maximum());
    }
    m_appending = false;
}
