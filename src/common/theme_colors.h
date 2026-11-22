/**
 * @file theme_colors.h
 * @brief 主题相关颜色（供 QPainter 自绘使用，QSS 无法控制自绘内容）
 * @details 全局函数，根据当前主题返回对应颜色。阵营颜色（红/蓝）是语义色，
 *          不随主题变化，仍由 dynamicdata.h 的 campColor() 提供。
 * @date 2026-11-22
 */

#ifndef THEME_COLORS_H
#define THEME_COLORS_H

#include <QColor>

#ifdef CPSS_COMMON_EXPORT
#   define THEME_COLORS_API __declspec(dllexport)
#else
#   define THEME_COLORS_API __declspec(dllimport)
#endif

namespace ThemeColors {

/** @brief 当前是否为黑夜主题 */
THEME_COLORS_API bool isDark();

/** @brief 普通文字颜色 */
THEME_COLORS_API QColor text();

/** @brief 次要/说明文字颜色（时间戳、标注） */
THEME_COLORS_API QColor textSecondary();

/** @brief 面板/控件背景色 */
THEME_COLORS_API QColor background();

/** @brief 网格线/分隔线颜色 */
THEME_COLORS_API QColor gridLine();

/** @brief 属性框背景色 */
THEME_COLORS_API QColor propertyBoxBackground();

/** @brief 属性框边框色 */
THEME_COLORS_API QColor propertyBoxBorder();

/** @brief 航迹线颜色 */
THEME_COLORS_API QColor trackLine();

/** @brief 选中高亮色 */
THEME_COLORS_API QColor highlight();

} // namespace ThemeColors

#endif // THEME_COLORS_H
