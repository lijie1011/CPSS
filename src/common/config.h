#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

class Config
{
public:
    static Config& instance();

    QString getChartPath() const;
    void setChartPath(const QString &path);

    int getWebPort() const;
    void setWebPort(int port);

    bool getAutoLoadCharts() const;
    void setAutoLoadCharts(bool enabled);

    QString getColorScheme() const;
    void setColorScheme(const QString &scheme);

    QString getDisplayCategory() const;
    void setDisplayCategory(const QString &category);

    void load();
    void save();

private:
    Config();
    ~Config();

    QSettings m_settings;
};

#endif
