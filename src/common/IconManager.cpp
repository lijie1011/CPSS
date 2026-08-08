/**
 * @file IconManager.cpp
 * @brief 图标管理器实现
 * @details 实现图标模板的自动转换、着色缓存等功能。
 *          启动时自动从 JB/ 目录生成白色模板到 JB_Souce/ 目录。
 */

#include "IconManager.h"
#include "logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>

/**
 * @brief 构造函数
 */
IconManager::IconManager()
    : m_initialized(false)
{
}

/**
 * @brief 析构函数
 */
IconManager::~IconManager()
{
}

/**
 * @brief 获取单例实例
 * @return IconManager 引用
 */
IconManager& IconManager::instance()
{
    static IconManager inst;
    return inst;
}

/**
 * @brief 获取资源目录路径
 * @return 资源目录绝对路径（exe目录/../resource）
 */
QString IconManager::resourcePath() const
{
    return QCoreApplication::applicationDirPath() + "/../resource";
}

/**
 * @brief 获取 JB_Souce 模板目录路径
 * @return 模板目录绝对路径
 */
QString IconManager::templatePath() const
{
    return resourcePath() + "/JB_Souce";
}

/**
 * @brief 获取 JB 源图标目录路径
 * @return 源图标目录绝对路径
 */
QString IconManager::sourcePath() const
{
    return resourcePath() + "/JB";
}

/**
 * @brief 初始化图标管理器
 * @details 确保模板存在，加载所有模板到内存
 */
void IconManager::init()
{
    if (m_initialized) return;

    Logger::info("[IconManager] 初始化开始...");
    ensureTemplatesExist();
    loadAllTemplates();
    m_initialized = true;
    Logger::info("[IconManager] 初始化完成，已加载 %d 个模板图标", m_templates.size());
}

/**
 * @brief 确保 JB_Souce 目录存在且包含模板文件
 * @details 若目录不存在或为空，自动从 JB/ 目录转换生成
 */
void IconManager::ensureTemplatesExist()
{
    QDir tpDir(templatePath());
    if (tpDir.exists() && !tpDir.entryList(QStringList() << "*.png", QDir::Files).isEmpty()) {
        Logger::info("[IconManager] JB_Souce 目录已存在，跳过转换");
        return;
    }

    Logger::info("[IconManager] JB_Souce 目录不存在或为空，从 JB/ 转换...");
    convertAllFromSource();
}

/**
 * @brief 从 JB/ 目录转换所有图标到 JB_Souce/
 * @details 遍历 JB/ 下所有 PNG，用 makeTemplate() 转为白色模板，
 *          保存到 JB_Souce/ 目录，保持相同文件名
 */
void IconManager::convertAllFromSource()
{
    QDir srcDir(sourcePath());
    if (!srcDir.exists()) {
        Logger::warn("[IconManager] 源目录不存在: %s",
                     sourcePath().toUtf8().constData());
        return;
    }

    QDir tpDir(templatePath());
    if (!tpDir.exists()) {
        tpDir.mkpath(".");
    }

    QStringList pngFiles = srcDir.entryList(QStringList() << "*.png", QDir::Files);
    int success = 0;

    for (const QString &fileName : pngFiles) {
        QString srcFile = srcDir.absoluteFilePath(fileName);
        QImage srcImg(srcFile);

        if (srcImg.isNull()) {
            Logger::warn("[IconManager] 无法加载: %s",
                         fileName.toUtf8().constData());
            continue;
        }

        QImage tmpl = makeTemplate(srcImg);
        QString dstFile = tpDir.absoluteFilePath(fileName);

        if (tmpl.save(dstFile, "PNG")) {
            Logger::info("[IconManager] 转换 OK: %s",
                         fileName.toUtf8().constData());
            success++;
        } else {
            Logger::warn("[IconManager] 转换失败: %s",
                         fileName.toUtf8().constData());
        }
    }

    Logger::info("[IconManager] 转换完成: %d/%d", success, pngFiles.size());
}

/**
 * @brief 加载所有模板图标到内存
 * @details 遍历 JB_Souce/ 下所有 PNG，加载到 m_templates map
 */
void IconManager::loadAllTemplates()
{
    QDir tpDir(templatePath());
    QStringList pngFiles = tpDir.entryList(QStringList() << "*.png", QDir::Files);

    for (const QString &fileName : pngFiles) {
        QString filePath = tpDir.absoluteFilePath(fileName);
        QImage img(filePath);

        if (!img.isNull()) {
            m_templates[fileName] = img;
        }
    }
}

/**
 * @brief 获取着色后的图标（带缓存）
 * @param iconName 图标文件名
 * @param color 目标颜色
 * @return 着色后的 QImage
 */
QImage IconManager::getIcon(const QString &iconName, const QColor &color)
{
    // 先查缓存
    CacheKey key(iconName, color.rgba());
    if (m_tintCache.contains(key)) {
        return m_tintCache[key];
    }

    // 获取模板
    QImage tmpl = getTemplate(iconName);
    if (tmpl.isNull()) {
        Logger::warn("[IconManager] 模板不存在: %s",
                     iconName.toUtf8().constData());
        return QImage();
    }

    // 着色并缓存
    QImage tinted = tintImage(tmpl, color);
    m_tintCache[key] = tinted;
    return tinted;
}

/**
 * @brief 获取模板图标（未着色）
 * @param iconName 图标文件名
 * @return 模板 QImage
 */
QImage IconManager::getTemplate(const QString &iconName) const
{
    if (m_templates.contains(iconName)) {
        return m_templates[iconName];
    }
    return QImage();
}

/**
 * @brief 静态方法：对图像进行着色
 * @param source 源图像（白色/灰度模板，带 alpha 通道）
 * @param color 目标颜色
 * @return 着色后的 QImage
 * @details 使用 CompositionMode_SourceIn 以 alpha 为遮罩填充颜色
 */
QImage IconManager::tintImage(const QImage &source, const QColor &color)
{
    if (source.isNull()) return QImage();

    // 确保格式为 ARGB32
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(result.rect(), color);
    painter.end();
    return result;
}

/**
 * @brief 静态方法：将彩色图像转为白色模板
 * @param source 源图像（彩色 PNG）
 * @return 白色模板 QImage
 * @details 遍历每个像素，将非透明像素的 RGB 设为白色 (255,255,255)，
 *          保留 alpha 通道不变。透明区域保持透明。
 */
QImage IconManager::makeTemplate(const QImage &source)
{
    if (source.isNull()) return QImage();

    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < result.height(); y++) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); x++) {
            int alpha = qAlpha(line[x]);
            if (alpha > 0) {
                // 有颜色的区域设为白色，保留 alpha
                line[x] = qRgba(255, 255, 255, alpha);
            }
            // alpha == 0 的区域保持透明（不变）
        }
    }
    return result;
}