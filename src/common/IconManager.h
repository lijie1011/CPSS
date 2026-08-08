/**
 * @file IconManager.h
 * @brief 图标管理器 - 运行时着色与缓存
 * @details 负责将 JB_Souce/ 下的白色模板 PNG 按阵营颜色动态着色，
 *          并缓存结果避免重复计算。首次启动时若 JB_Souce/ 不存在，
 *          自动从 JB/ 目录转换生成白色模板。
 */

#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QImage>
#include <QColor>
#include <QMap>
#include <QString>
#include <QPair>
#include <QDir>

// 与 logger.h / xmlconfig.h 一致的导出宏：
// 编译 cpss_common 动态库时导出符号，其它模块引用时导入符号，
// 否则 IconManager 的成员函数不会从 DLL 导出，导致链接时 LNK2019。
#ifdef CPSS_COMMON_EXPORT
#   define CPSS_COMMON_API __declspec(dllexport)
#else
#   define CPSS_COMMON_API __declspec(dllimport)
#endif

/**
 * @class IconManager
 * @brief 图标管理器类
 * @details 单例模式，管理平台图标的加载、着色和缓存。
 *          启动时自动将 JB/ 的彩色图标转为 JB_Souce/ 的白色模板，
 *          运行时按需着色并缓存，避免重复计算。
 */
class CPSS_COMMON_API IconManager
{
public:
    /**
     * @brief 获取单例实例
     * @return IconManager 引用
     */
    static IconManager& instance();

    /**
     * @brief 初始化图标管理器
     * @details 检查并创建 JB_Souce/ 目录，加载所有模板图标
     */
    void init();

    /**
     * @brief 获取着色后的图标（带缓存）
     * @param iconName 图标文件名（如 "驱逐舰.png"）
     * @param color 目标颜色
     * @return 着色后的 QImage，若图标不存在返回空 QImage
     */
    QImage getIcon(const QString &iconName, const QColor &color);

    /**
     * @brief 获取模板图标（未着色）
     * @param iconName 图标文件名
     * @return 模板 QImage
     */
    QImage getTemplate(const QString &iconName) const;

    /**
     * @brief 静态方法：对图像进行着色
     * @param source 源图像（白色/灰度模板，带 alpha 通道）
     * @param color 目标颜色
     * @return 着色后的 QImage
     * @details 使用 QPainter::CompositionMode_SourceIn，
     *          以 alpha 通道为遮罩填充目标颜色，透明区域保持不变
     */
    static QImage tintImage(const QImage &source, const QColor &color);

    /**
     * @brief 静态方法：将彩色图像转为白色模板
     * @param source 源图像（彩色 PNG）
     * @return 白色模板 QImage（有颜色的像素变为白色，保留 alpha）
     */
    static QImage makeTemplate(const QImage &source);

    /**
     * @brief 获取资源目录路径
     * @return 资源目录绝对路径
     */
    QString resourcePath() const;

    /**
     * @brief 获取 JB_Souce 模板目录路径
     * @return 模板目录绝对路径
     */
    QString templatePath() const;

    /**
     * @brief 获取 JB 源图标目录路径
     * @return 源图标目录绝对路径
     */
    QString sourcePath() const;

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    IconManager();

    /**
     * @brief 析构函数
     */
    ~IconManager();

    IconManager(const IconManager&) = delete;
    IconManager& operator=(const IconManager&) = delete;

    /**
     * @brief 确保 JB_Souce 目录存在且包含模板文件
     * @details 若目录不存在，自动从 JB/ 目录转换生成
     */
    void ensureTemplatesExist();

    /**
     * @brief 从 JB/ 目录转换所有图标到 JB_Souce/
     */
    void convertAllFromSource();

    /**
     * @brief 加载所有模板图标到内存
     */
    void loadAllTemplates();

    /** 图标缓存键：文件名 + 颜色 RGBA 值 */
    using CacheKey = QPair<QString, QRgb>;

    QMap<QString, QImage> m_templates;  ///< 模板图标缓存（文件名 → QImage）
    QMap<CacheKey, QImage> m_tintCache; ///< 着色图标缓存（键 → QImage）
    bool m_initialized;                 ///< 是否已初始化
};

#endif // ICONMANAGER_H