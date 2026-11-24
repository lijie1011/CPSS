/**
 * @file ReplayWidget.h
 * @brief 历史数据回放界面
 * @date 2026-11-23
 */

#ifndef REPLAYWIDGET_H
#define REPLAYWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include "recorder.h"
#include "forwardmanager.h"

class ReplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReplayWidget(QWidget *parent = nullptr);
    ~ReplayWidget();

private slots:
    void onStartRecord();
    void onStopRecord();
    void onRefreshFiles();
    void onDeleteFile();
    void onOpenDir();
    void onLoadFile();
    void onPlayPause();
    void onStopPlay();
    void onSeek(int value);
    void onSpeedChanged(int index);
    void onReplaySourcesChanged();
    void onRecordStatus(const QString &msg);
    void onReplayStatus(const QString &msg);
    void onPositionChanged(qint64 ms, int injected);
    void onPlaybackFinished();
    void onRecordTimer();
    void onForwardModeChanged(int id);
    void onUdpSubModeChanged(int id);
    void onForwardData(const ForwardPayload &payload);
    void onForwardFormatChanged(int id);
    void onByteOrderChanged(int id);

private:
    void loadForwardConfig();
    void saveForwardConfig();
    QJsonObject currentForwardConfig() const;

private:
    QString recordDir() const;
    void updateFileList();
    void updatePlaybackUI();
    void applyReplaySourceFilter();

    RecordWriter m_recorder;
    ReplayPlayer m_player;
    QTimer m_uiTimer;          ///< 录制时刷新状态标签

    // 录制区
    QPushButton *m_startRecBtn;
    QPushButton *m_stopRecBtn;
    QComboBox *m_sampleCombo;
    QCheckBox *m_srcRedisCb;
    QCheckBox *m_srcDdsCb;
    QCheckBox *m_srcHttpCb;
    QComboBox *m_campCombo;
    QLineEdit *m_typeEdit;
    QLabel *m_recStatusLabel;

    // 文件列表区
    QListWidget *m_fileList;
    QPushButton *m_refreshBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_openDirBtn;

    // 回放区
    QPushButton *m_playBtn;
    QPushButton *m_stopPlayBtn;
    QSlider *m_timeSlider;
    QLabel *m_timeLabel;
    QComboBox *m_speedCombo;
    QCheckBox *m_rpRedisCb;
    QCheckBox *m_rpDdsCb;
    QCheckBox *m_rpHttpCb;
    QLabel *m_playStatusLabel;

    // 转发设置
    ForwardManager m_forwardManager;
    QButtonGroup *m_forwardGroup;      ///< Off/Dds/Udp/Tcp 主单选
    QButtonGroup *m_formatGroup;       ///< JSON/二进制 格式单选
    QButtonGroup *m_byteOrderGroup;    ///< 大端/小端 字节序单选
    QButtonGroup *m_udpSubGroup;       ///< UDP 单播/组播子单选
    QStackedWidget *m_forwardStack;    ///< 按主单选切换配置面板
    QStackedWidget *m_udpStack;        ///< UDP 单播/组播面板
    QLabel *m_forwardStatusLabel;

    // DDS 配置
    QLineEdit *m_ddsDomainEdit;
    QLineEdit *m_ddsBindEdit;
    QLineEdit *m_ddsPlatformTopicEdit;
    QLineEdit *m_ddsEventTopicEdit;

    // UDP 配置
    QLineEdit *m_udpTargetIpEdit;
    QLineEdit *m_udpMcastAddrEdit;
    QLineEdit *m_udpPortEdit;
    QLineEdit *m_udpTtlEdit;
    QComboBox *m_udpIfCombo;
    QCheckBox *m_udpLoopbackCb;

    // TCP 配置
    QLineEdit *m_tcpIpEdit;
    QLineEdit *m_tcpPortEdit;
    QLineEdit *m_tcpReconnectEdit;

    bool m_playing = false;
};

#endif // REPLAYWIDGET_H
