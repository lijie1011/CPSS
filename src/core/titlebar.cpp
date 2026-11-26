/**
 * @file titlebar.cpp
 * @brief 自绘标题栏实现：拖动 / 双击最大化 / 最小化 / 关闭
 * @date 2026-11-26
 */

#include "titlebar.h"

#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("TitleBar");
    setFixedHeight(36);

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(10, 0, 4, 0);
    lay->setSpacing(8);

    // 左侧：程序标识（用小色块 + 文字，无图片依赖）
    QLabel *logo = new QLabel(QStringLiteral("CPSS"), this);
    logo->setObjectName("TitleLogo");
    logo->setFixedWidth(54);
    lay->addWidget(logo);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("TitleText");
    m_titleLabel->setText(parent ? parent->windowTitle() : QString());
    lay->addWidget(m_titleLabel, 1);

    // 右侧：最小化 / 最大化 / 关闭
    m_minBtn = new QToolButton(this);
    m_minBtn->setObjectName("TitleMin");
    m_minBtn->setText(QStringLiteral("─"));
    m_minBtn->setToolTip(QStringLiteral("最小化"));
    connect(m_minBtn, &QToolButton::clicked, this, [this]() { window()->showMinimized(); });
    lay->addWidget(m_minBtn);

    m_maxBtn = new QToolButton(this);
    m_maxBtn->setObjectName("TitleMax");
    m_maxBtn->setText(QStringLiteral("□"));
    m_maxBtn->setToolTip(QStringLiteral("最大化/还原"));
    connect(m_maxBtn, &QToolButton::clicked, this, [this]() {
        QWidget *w = window();
        if (w->isMaximized()) { w->showNormal(); m_maxBtn->setText(QStringLiteral("□")); }
        else { w->showMaximized(); m_maxBtn->setText(QStringLiteral("❐")); }
    });
    lay->addWidget(m_maxBtn);

    m_closeBtn = new QToolButton(this);
    m_closeBtn->setObjectName("TitleClose");
    m_closeBtn->setText(QStringLiteral("✕"));
    m_closeBtn->setToolTip(QStringLiteral("关闭"));
    connect(m_closeBtn, &QToolButton::clicked, this, [this]() { window()->close(); });
    lay->addWidget(m_closeBtn);

    setMouseTracking(false);
}

void TitleBar::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        // 用全局坐标差计算，避免窗口位置跳变
        m_dragPos = event->globalPos() - window()->frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && window() && !(window()->windowState() & Qt::WindowMaximized)) {
        window()->move(event->globalPos() - m_dragPos);
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_maxBtn->click();   // 双击标题栏 = 最大化/还原
    }
    QWidget::mouseDoubleClickEvent(event);
}
