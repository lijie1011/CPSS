/**
 * @file xmlconfig.cpp
 * @brief XML配置管理器实现
 * @details 使用QDomDocument读取/写入cpss.xml。当文件或节点缺失时
 *          回退到默认值。文件路径为<应用目录>/cpss.xml。
 */

#include "xmlconfig.h"
#include "logger.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>

static const int DEFAULT_CLOSE_MODE = 1;          ///< 默认插件关闭模式
static const char *TAG_CPSSS = "cpss";             ///< XML根节点标签
static const char *TAG_PLUGIN_CLOSE = "plugin";    ///< 插件节点标签
static const char *TAG_PLUGINS = "plugins";        ///< 插件清单集合标签
static const char *ATTR_CLOSE_MODE = "closeMode";  ///< 关闭模式属性名
static const char *ATTR_ID = "id";                 ///< 标识符属性名
static const char *ATTR_NAME = "name";             ///< 名称属性名
static const char *ATTR_VERSION = "version";       ///< 版本属性名

/**
 * @brief 构造函数
 * @details 初始化XML配置文件路径和默认关闭模式，并加载配置
 */
XmlConfig::XmlConfig()
    : m_filePath(QCoreApplication::applicationDirPath() + "/cpss.xml"),
      m_defaultCloseMode(DEFAULT_CLOSE_MODE)
{
    load();
}

/**
 * @brief 析构函数
 * @details 保存配置到XML文件
 */
XmlConfig::~XmlConfig()
{
    save();
}

/**
 * @brief 获取XML配置单例实例
 * @return XmlConfig引用
 */
XmlConfig& XmlConfig::instance()
{
    static XmlConfig instance;
    return instance;
}

/**
 * @brief 获取指定插件的关闭模式
 * @param pluginId 插件唯一标识符
 * @return 关闭模式值，若未配置则返回默认模式
 */
int XmlConfig::pluginCloseMode(const QString &pluginId) const
{
    auto it = m_pluginCloseModes.constFind(pluginId);
    if (it != m_pluginCloseModes.constEnd()) {
        return it.value();
    }
    return m_defaultCloseMode;
}

/**
 * @brief 获取默认插件关闭模式
 * @return 默认关闭模式值
 */
int XmlConfig::defaultPluginCloseMode() const
{
    return m_defaultCloseMode;
}

/**
 * @brief 设置指定插件的关闭模式
 * @param pluginId 插件唯一标识符
 * @param mode 关闭模式值
 */
void XmlConfig::setPluginCloseMode(const QString &pluginId, int mode)
{
    m_pluginCloseModes[pluginId] = mode;
}

/**
 * @brief 设置默认插件关闭模式
 * @param mode 默认关闭模式值
 */
void XmlConfig::setDefaultPluginCloseMode(int mode)
{
    m_defaultCloseMode = mode;
}

/**
 * @brief 更新插件清单
 * @param plugins 插件信息列表
 * @details 将最新的插件列表保存到配置并持久化到XML文件
 */
void XmlConfig::updatePluginManifest(const QList<PluginInfo> &plugins)
{
    m_pluginManifest = plugins;
    Logger::info("XmlConfig: updated manifest, %d plugin(s)", plugins.size());
    save();
}

/**
 * @brief 获取当前插件清单
 * @return 插件信息列表
 */
QList<XmlConfig::PluginInfo> XmlConfig::pluginManifest() const
{
    return m_pluginManifest;
}

/**
 * @brief 从XML文件加载配置
 * @details 解析cpss.xml文件，读取默认关闭模式和各插件的独立关闭模式覆盖值。
 *          若文件不存在或解析失败，则回退到默认值。
 */
void XmlConfig::load()
{
    m_defaultCloseMode = DEFAULT_CLOSE_MODE;
    m_pluginCloseModes.clear();

    Logger::info("XmlConfig: loading %s", m_filePath.toUtf8().constData());

    QFile file(m_filePath);
    if (!file.exists()) {
        Logger::info("XmlConfig: file not found, using defaults (closeMode=%d)", DEFAULT_CLOSE_MODE);
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::warn("XmlConfig: cannot open %s", m_filePath.toUtf8().constData());
        return;
    }

    QDomDocument doc;
    QString errMsg;
    int errLine = 0, errCol = 0;
    if (!doc.setContent(&file, &errMsg, &errLine, &errCol)) {
        file.close();
        Logger::warn("XmlConfig: parse error at line %d col %d: %s",
                     errLine, errCol, errMsg.toUtf8().constData());
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != TAG_CPSSS) {
        Logger::warn("XmlConfig: invalid root tag '%s', expected 'cpss'",
                     root.tagName().toUtf8().constData());
        return;
    }

    // 关闭模式节
    QDomElement pluginNode = root.firstChildElement(TAG_PLUGIN_CLOSE);
    if (!pluginNode.isNull()) {
        if (pluginNode.hasAttribute(ATTR_CLOSE_MODE)) {
            m_defaultCloseMode = pluginNode.attribute(ATTR_CLOSE_MODE).toInt();
        }

        QDomElement child = pluginNode.firstChildElement(TAG_PLUGIN_CLOSE);
        while (!child.isNull()) {
            QString id = child.attribute(ATTR_ID);
            if (!id.isEmpty() && child.hasAttribute(ATTR_CLOSE_MODE)) {
                m_pluginCloseModes[id] = child.attribute(ATTR_CLOSE_MODE).toInt();
            }
            child = child.nextSiblingElement(TAG_PLUGIN_CLOSE);
        }
    }

    Logger::info("XmlConfig: loaded defaultCloseMode=%d, pluginOverrides=%d",
                 m_defaultCloseMode, m_pluginCloseModes.size());
}

/**
 * @brief 将配置保存到XML文件
 * @details 将默认关闭模式、各插件关闭模式覆盖值以及插件清单写入cpss.xml。
 *          使用UTF-8编码，缩进格式化为2空格。
 */
void XmlConfig::save()
{
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());

    QDomDocument doc;
    QDomElement root = doc.createElement(TAG_CPSSS);
    doc.appendChild(root);

    // 关闭模式节
    QDomElement closeNode = doc.createElement(TAG_PLUGIN_CLOSE);
    closeNode.setAttribute(ATTR_CLOSE_MODE, QString::number(m_defaultCloseMode));
    root.appendChild(closeNode);

    for (auto it = m_pluginCloseModes.constBegin();
         it != m_pluginCloseModes.constEnd(); ++it) {
        QDomElement child = doc.createElement(TAG_PLUGIN_CLOSE);
        child.setAttribute(ATTR_ID, it.key());
        child.setAttribute(ATTR_CLOSE_MODE, QString::number(it.value()));
        closeNode.appendChild(child);
    }

    // 清单节
    if (!m_pluginManifest.isEmpty()) {
        QDomElement manifestNode = doc.createElement(TAG_PLUGINS);
        for (const PluginInfo &info : m_pluginManifest) {
            QDomElement p = doc.createElement(TAG_PLUGIN_CLOSE);
            p.setAttribute(ATTR_ID, info.id);
            p.setAttribute(ATTR_NAME, info.name);
            p.setAttribute(ATTR_VERSION, info.version);
            manifestNode.appendChild(p);
        }
        root.appendChild(manifestNode);
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::warn("XmlConfig: cannot write %s", m_filePath.toUtf8().constData());
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << doc.toString(2);
    file.close();
    Logger::info("XmlConfig: saved %s", m_filePath.toUtf8().constData());
}
