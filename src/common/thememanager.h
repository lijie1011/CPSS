/**
 * @file thememanager.h
 * @brief 主题管理器单例（白天/黑夜）
 * @details 职责：
 *          1. 加载 qss（:/themes/day.qss 或 night.qss）到 qApp；
 *          2. 更新 ThemeColors 自绘颜色；
 *          3. 保存主题到 Config；
 *          4. 发 themeChanged 信号，让自绘控件（海图等）响应重绘。
 *
 *          Enclib 海图配色由 chartplugin 监听 themeChanged 后调用
 *          EnclDrawSetColorScheme()，避免 common 依赖第三方库。
 * @date 2026-11-22
 */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

#ifdef CPSS_COMMON_EXPORT
#   define THEME_API __declspec(dllexport)
#else
#   define THEME_API __declspec(dllimport)
#endif

/**
 * @enum Theme
 * @brief 主题类型（仅两档）
 */
enum class Theme {
    Day,
    Night
};

/**
 * @class ThemeManager
 * @brief 主题管理器单例
 */
class THEME_API ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();

    /** @brief 初始化：从 Config 读取当前主题并应用（程序启动时调用一次） */
    void initialize();

    /** @brief 应用指定主题 */
    void apply(Theme theme);

    /** @brief 切换为白天/黑夜（toggle） */
    void toggle();

    Theme current() const { return m_current; }
    bool isDark() const { return m_current == Theme::Night; }

    /** @brief 主题名字符串（DAY/NIGHT），供 Enclib 等使用 */
    static QString themeName(Theme theme);

signals:
    /** @brief 主题变化（自绘控件连接此信号后 update() 重绘） */
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();

    static ThemeManager *s_instance;
    Theme m_current;
};

#endif // THEMEMANAGER_H
