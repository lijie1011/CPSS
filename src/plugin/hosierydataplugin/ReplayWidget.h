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
#include "recorder.h"

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

private:
    QString recordDir() const;
    void updateFileList();
    void updatePlaybackUI();
    void applyReplaySourceFilter();

    RecordWriter m_recorder;
    ReplayPlayer m_player;

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

    bool m_playing = false;
    int m_lastInjected = 0;
};

#endif // REPLAYWIDGET_H
