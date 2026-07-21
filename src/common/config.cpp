#include "config.h"
#include <QCoreApplication>

Config::Config()
    : m_settings(QCoreApplication::applicationDirPath() + "/cpss.ini",
                 QSettings::IniFormat)
{
    load();
}

Config::~Config()
{
    save();
}

Config& Config::instance()
{
    static Config instance;
    return instance;
}

QString Config::getChartPath() const
{
    return m_settings.value("Chart/Path", "3dParty/Enclib").toString();
}

void Config::setChartPath(const QString &path)
{
    m_settings.setValue("Chart/Path", path);
}

int Config::getWebPort() const
{
    return m_settings.value("Web/Port", 12345).toInt();
}

void Config::setWebPort(int port)
{
    m_settings.setValue("Web/Port", port);
}

bool Config::getAutoLoadCharts() const
{
    return m_settings.value("Chart/AutoLoad", true).toBool();
}

void Config::setAutoLoadCharts(bool enabled)
{
    m_settings.setValue("Chart/AutoLoad", enabled);
}

QString Config::getColorScheme() const
{
    return m_settings.value("Display/ColorScheme", "DAY").toString();
}

void Config::setColorScheme(const QString &scheme)
{
    m_settings.setValue("Display/ColorScheme", scheme);
}

QString Config::getDisplayCategory() const
{
    return m_settings.value("Display/Category", "STANDARD").toString();
}

void Config::setDisplayCategory(const QString &category)
{
    m_settings.setValue("Display/Category", category);
}

void Config::load()
{
}

void Config::save()
{
    m_settings.sync();
}
