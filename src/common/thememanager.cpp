/**
 * @file thememanager.cpp
 * @brief 主题管理器实现
 * @date 2026-11-22
 */

#include "thememanager.h"
#include "config.h"
#include "logger.h"

#include <QApplication>
#include <QFile>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent),
      m_current(Theme::Day)
{
}

ThemeManager::~ThemeManager()
{
}

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) s_instance = new ThemeManager();
    return s_instance;
}

QString ThemeManager::themeName(Theme theme)
{
    return theme == Theme::Night ? QStringLiteral("NIGHT") : QStringLiteral("DAY");
}

void ThemeManager::initialize()
{
    const QString saved = Config::instance().getColorScheme();
    Theme theme = (saved.compare(QStringLiteral("NIGHT"), Qt::CaseInsensitive) == 0)
                  ? Theme::Night : Theme::Day;
    apply(theme);
    Logger::info("[ThemeManager] initialized: %s", themeName(theme).toUtf8().constData());
}

void ThemeManager::apply(Theme theme)
{
    m_current = theme;

    // 1) 加载 QSS 到 qApp
    const QString qssPath = (theme == Theme::Night)
                            ? QStringLiteral(":/themes/night.qss")
                            : QStringLiteral(":/themes/day.qss");
    QFile file(qssPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString style = QString::fromUtf8(file.readAll());
        qApp->setStyleSheet(style);
        file.close();
    } else {
        Logger::warn("[ThemeManager] cannot open qss: %s", qssPath.toUtf8().constData());
    }

    // 2) 保存到 Config
    Config::instance().setColorScheme(themeName(theme));

    // 3) 通知自绘控件重绘（含 chartplugin 同步 Enclib 配色）
    emit themeChanged(theme);

    Logger::info("[ThemeManager] applied: %s", themeName(theme).toUtf8().constData());
}

void ThemeManager::toggle()
{
    apply(m_current == Theme::Day ? Theme::Night : Theme::Day);
}
