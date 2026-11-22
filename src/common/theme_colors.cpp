/**
 * @file theme_colors.cpp
 * @brief 主题颜色实现（根据当前主题返回自绘颜色）
 * @date 2026-11-22
 */

#include "theme_colors.h"
#include "thememanager.h"

bool ThemeColors::isDark()
{
    return ThemeManager::instance()->isDark();
}

QColor ThemeColors::text()
{
    return isDark() ? QColor("#D4D4D4") : QColor("#222222");
}

QColor ThemeColors::textSecondary()
{
    return isDark() ? QColor("#858585") : QColor("#888888");
}

QColor ThemeColors::background()
{
    return isDark() ? QColor("#1E1E1E") : QColor("#FFFFFF");
}

QColor ThemeColors::gridLine()
{
    return isDark() ? QColor("#3C3C3C") : QColor("#E0E0E0");
}

QColor ThemeColors::propertyBoxBackground()
{
    return isDark() ? QColor("#2D2D30") : QColor("#FAFAFA");
}

QColor ThemeColors::propertyBoxBorder()
{
    return isDark() ? QColor("#555555") : QColor("#B0B0B0");
}

QColor ThemeColors::trackLine()
{
    return isDark() ? QColor("#4EC9B0") : QColor("#2E8B57");
}

QColor ThemeColors::highlight()
{
    return isDark() ? QColor("#094771") : QColor("#3399FF");
}
