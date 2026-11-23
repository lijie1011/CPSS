/**
 * @file ReplayWidget.cpp
 * @brief 历史数据回放界面实现
 * @date 2026-11-23
 */

#include "ReplayWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTime>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>

ReplayWidget::ReplayWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("历史数据回放"));

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(8, 8, 8, 8);
    main->setSpacing(8);

    // ========== 录制区 ==========
    QGroupBox *recGroup = new QGroupBox(QStringLiteral("录制"), this);
    QVBoxLayout *rec = new QVBoxLayout(recGroup);

    QHBoxLayout *recRow1 = new QHBoxLayout();
    m_startRecBtn = new QPushButton(QStringLiteral("开始录制"), this);
    m_stopRecBtn = new QPushButton(QStringLiteral("停止"), this);
    m_stopRecBtn->setEnabled(false);
    recRow1->addWidget(m_startRecBtn);
    recRow1->addWidget(m_stopRecBtn);
    recRow1->addWidget(new QLabel(QStringLiteral("采样:"), this));
    m_sampleCombo = new QComboBox(this);
    m_sampleCombo->addItem(QStringLiteral("全量"), 1);
    m_sampleCombo->addItem(QStringLiteral("1/2"), 2);
    m_sampleCombo->addItem(QStringLiteral("1/5"), 5);
    m_sampleCombo->addItem(QStringLiteral("1/10"), 10);
    recRow1->addWidget(m_sampleCombo);
    recRow1->addStretch();
    rec->addLayout(recRow1);

    QHBoxLayout *recRow2 = new QHBoxLayout();
    recRow2->addWidget(new QLabel(QStringLiteral("数据源:"), this));
    m_srcRedisCb = new QCheckBox(QStringLiteral("Redis"), this);
    m_srcDdsCb = new QCheckBox(QStringLiteral("DDS"), this);
    m_srcHttpCb = new QCheckBox(QStringLiteral("HTTP"), this);
    m_srcRedisCb->setChecked(true);
    m_srcDdsCb->setChecked(true);
    m_srcRedisCb->setEnabled(false);   // 第一版只允许 Redis/DDS（HTTP 未启用）
    m_srcDdsCb->setEnabled(false);
    m_srcHttpCb->setEnabled(false);
    recRow2->addWidget(m_srcRedisCb);
    recRow2->addWidget(m_srcDdsCb);
    recRow2->addWidget(m_srcHttpCb);
    recRow2->addWidget(new QLabel(QStringLiteral("阵营:"), this));
    m_campCombo = new QComboBox(this);
    m_campCombo->addItem(QStringLiteral("全部"), -1);
    m_campCombo->addItem(QStringLiteral("红方"), Camp_Mine);
    m_campCombo->addItem(QStringLiteral("蓝方"), Camp_Enemy);
    recRow2->addWidget(m_campCombo);
    recRow2->addWidget(new QLabel(QStringLiteral("类型:"), this));
    m_typeEdit = new QLineEdit(this);
    m_typeEdit->setPlaceholderText(QStringLiteral("留空=全部"));
    m_typeEdit->setMaximumWidth(120);
    recRow2->addWidget(m_typeEdit);
    recRow2->addStretch();
    rec->addLayout(recRow2);

    m_recStatusLabel = new QLabel(QStringLiteral("状态: 空闲"), this);
    rec->addWidget(m_recStatusLabel);
    main->addWidget(recGroup);

    // ========== 文件列表区 ==========
    QGroupBox *listGroup = new QGroupBox(QStringLiteral("录制文件"), this);
    QVBoxLayout *list = new QVBoxLayout(listGroup);
    m_fileList = new QListWidget(this);
    list->addWidget(m_fileList);
    QHBoxLayout *listBtns = new QHBoxLayout();
    m_refreshBtn = new QPushButton(QStringLiteral("刷新"), this);
    m_deleteBtn = new QPushButton(QStringLiteral("删除选中"), this);
    m_openDirBtn = new QPushButton(QStringLiteral("打开目录"), this);
    listBtns->addWidget(m_refreshBtn);
    listBtns->addWidget(m_deleteBtn);
    listBtns->addWidget(m_openDirBtn);
    listBtns->addStretch();
    list->addLayout(listBtns);
    main->addWidget(listGroup);

    // ========== 回放区 ==========
    QGroupBox *playGroup = new QGroupBox(QStringLiteral("回放"), this);
    QVBoxLayout *play = new QVBoxLayout(playGroup);

    QHBoxLayout *playRow1 = new QHBoxLayout();
    m_playBtn = new QPushButton(QStringLiteral("播放"), this);
    m_stopPlayBtn = new QPushButton(QStringLiteral("停止"), this);
    m_stopPlayBtn->setEnabled(false);
    playRow1->addWidget(m_playBtn);
    playRow1->addWidget(m_stopPlayBtn);
    playRow1->addWidget(new QLabel(QStringLiteral("速度:"), this));
    m_speedCombo = new QComboBox(this);
    m_speedCombo->addItem(QStringLiteral("0.5x"), 1);
    m_speedCombo->addItem(QStringLiteral("1x"), 2);
    m_speedCombo->addItem(QStringLiteral("2x"), 4);
    m_speedCombo->addItem(QStringLiteral("4x"), 8);
    m_speedCombo->setCurrentIndex(1);
    playRow1->addWidget(m_speedCombo);
    playRow1->addStretch();
    play->addLayout(playRow1);

    QHBoxLayout *playRow2 = new QHBoxLayout();
    playRow2->addWidget(new QLabel(QStringLiteral("回放源:"), this));
    m_rpRedisCb = new QCheckBox(QStringLiteral("Redis"), this);
    m_rpDdsCb = new QCheckBox(QStringLiteral("DDS"), this);
    m_rpHttpCb = new QCheckBox(QStringLiteral("HTTP"), this);
    m_rpRedisCb->setChecked(true);
    m_rpDdsCb->setChecked(true);
    m_rpHttpCb->setChecked(true);
    playRow2->addWidget(m_rpRedisCb);
    playRow2->addWidget(m_rpDdsCb);
    playRow2->addWidget(m_rpHttpCb);
    playRow2->addStretch();
    play->addLayout(playRow2);

    m_timeSlider = new QSlider(Qt::Horizontal, this);
    m_timeSlider->setRange(0, 0);
    play->addWidget(m_timeSlider);

    m_timeLabel = new QLabel(QStringLiteral("00:00 / 00:00"), this);
    play->addWidget(m_timeLabel);
    m_playStatusLabel = new QLabel(QStringLiteral("状态: 未加载"), this);
    play->addWidget(m_playStatusLabel);
    main->addWidget(playGroup);

    // ===== 连接信号 =====
    connect(m_startRecBtn, &QPushButton::clicked, this, &ReplayWidget::onStartRecord);
    connect(m_stopRecBtn, &QPushButton::clicked, this, &ReplayWidget::onStopRecord);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ReplayWidget::onRefreshFiles);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ReplayWidget::onDeleteFile);
    connect(m_openDirBtn, &QPushButton::clicked, this, &ReplayWidget::onOpenDir);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &ReplayWidget::onLoadFile);
    connect(m_playBtn, &QPushButton::clicked, this, &ReplayWidget::onPlayPause);
    connect(m_stopPlayBtn, &QPushButton::clicked, this, &ReplayWidget::onStopPlay);
    connect(m_timeSlider, &QSlider::sliderMoved, this, &ReplayWidget::onSeek);
    connect(m_speedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReplayWidget::onSpeedChanged);
    connect(m_rpRedisCb, &QCheckBox::toggled, this, &ReplayWidget::onReplaySourcesChanged);
    connect(m_rpDdsCb, &QCheckBox::toggled, this, &ReplayWidget::onReplaySourcesChanged);
    connect(m_rpHttpCb, &QCheckBox::toggled, this, &ReplayWidget::onReplaySourcesChanged);

    connect(&m_recorder, &RecordWriter::statusChanged, this, &ReplayWidget::onRecordStatus);
    connect(&m_player, &ReplayPlayer::statusChanged, this, &ReplayWidget::onReplayStatus);
    connect(&m_player, &ReplayPlayer::positionChanged, this, &ReplayWidget::onPositionChanged);
    connect(&m_player, &ReplayPlayer::playbackFinished, this, &ReplayWidget::onPlaybackFinished);

    updateFileList();
}

ReplayWidget::~ReplayWidget()
{
    m_recorder.stop();
    m_player.unload();
}

QString ReplayWidget::recordDir() const
{
    return QCoreApplication::applicationDirPath() + "/records";
}

void ReplayWidget::onStartRecord()
{
    if (m_recorder.isRecording()) return;
    const QString dir = recordDir();
    QDir().mkpath(dir);
    const QString name = QStringLiteral("rec_%1.cpssrec")
                         .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString path = dir + "/" + name;

    int sampleDiv = m_sampleCombo->currentData().toInt();
    QSet<int> sources;
    if (m_srcRedisCb->isChecked()) sources.insert(Protocol_Redis);
    if (m_srcDdsCb->isChecked()) sources.insert(Protocol_DDS);
    if (m_srcHttpCb->isChecked()) sources.insert(Protocol_HTTP);
    if (sources.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请至少选择一个数据源"));
        return;
    }

    int camp = m_campCombo->currentData().toInt();
    QString type = m_typeEdit->text().trimmed();

    if (m_recorder.start(path, sampleDiv, sources, camp, type)) {
        // 明确的状态反馈：按钮文字变化 + 禁用 + 红色样式
        m_startRecBtn->setText(QStringLiteral("● 录制中..."));
        m_startRecBtn->setEnabled(false);
        m_startRecBtn->setStyleSheet("QPushButton { color: #FFFFFF; background-color: #C00000; }");
        m_stopRecBtn->setEnabled(true);
        m_recStatusLabel->setText(QStringLiteral("状态: 录制中"));
    }
}

void ReplayWidget::onStopRecord()
{
    const QString path = m_recorder.lastFilePath();
    const qint64 count = m_recorder.recordedCount();
    m_recorder.stop();

    // 0 条记录 = 空文件，直接删除，避免产生不可播放的无效文件
    if (count <= 0 && !path.isEmpty() && QFile::exists(path)) {
        QFile::remove(path);
    }

    // 恢复按钮状态
    m_startRecBtn->setText(QStringLiteral("开始录制"));
    m_startRecBtn->setEnabled(true);
    m_startRecBtn->setStyleSheet(QString());
    m_stopRecBtn->setEnabled(false);
    m_recStatusLabel->setText(QStringLiteral("状态: 开始录制"));
    updateFileList();
}

void ReplayWidget::onRefreshFiles()
{
    updateFileList();
}

void ReplayWidget::updateFileList()
{
    m_fileList->clear();
    const QDir dir(recordDir());
    if (!dir.exists()) return;
    const QStringList files = dir.entryList(QStringList() << "*.cpssrec", QDir::Files, QDir::Name | QDir::Reversed);
    for (const QString &f : files) {
        const RecFileInfo info = RecFileReader::fileInfo(dir.filePath(f));
        // 跳过 0KB / 无有效记录的空文件，避免加载后无法播放
        if (info.fileSize <= 128 || info.recordCount <= 0) continue;
        const QString dur = QTime(0, 0).addMSecs(info.durationMs).toString(QStringLiteral("mm:ss"));
        const QString size = QStringLiteral("%1KB").arg(info.fileSize / 1024);
        QString src;
        for (int s : info.sources) {
            if (s == Protocol_Redis) src += "Redis ";
            else if (s == Protocol_DDS) src += "DDS ";
            else if (s == Protocol_HTTP) src += "HTTP ";
        }
        QListWidgetItem *item = new QListWidgetItem(
            QStringLiteral("%1  %2  %3条  %4  [%5]")
            .arg(f, dur).arg(info.recordCount).arg(size, src.trimmed()));
        item->setData(Qt::UserRole, dir.filePath(f));
        m_fileList->addItem(item);
    }
}

void ReplayWidget::onDeleteFile()
{
    QListWidgetItem *item = m_fileList->currentItem();
    if (!item) return;
    const QString path = item->data(Qt::UserRole).toString();
    if (QMessageBox::question(this, QStringLiteral("确认"),
                              QStringLiteral("删除 %1 ?").arg(QFileInfo(path).fileName())) == QMessageBox::Yes) {
        QFile::remove(path);
        updateFileList();
    }
}

void ReplayWidget::onOpenDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(recordDir()));
}

void ReplayWidget::onLoadFile()
{
    QListWidgetItem *item = m_fileList->currentItem();
    if (!item) return;
    const QString path = item->data(Qt::UserRole).toString();
    if (m_recorder.isRecording()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先停止录制"));
        return;
    }
    if (m_player.load(path)) {
        m_lastInjected = 0;
        m_timeSlider->setRange(0, (int)m_player.durationMs());
        m_timeSlider->setValue(0);
        m_stopPlayBtn->setEnabled(true);
        applyReplaySourceFilter();
        updatePlaybackUI();
    }
}

void ReplayWidget::onPlayPause()
{
    // 未加载文件时给出明确提示（避免点了没反应）
    if (m_player.recordCount() == 0) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先双击文件列表中的录制文件加载，再点击播放"));
        return;
    }
    if (m_playing) {
        m_player.pause();
        m_playing = false;
        m_playBtn->setText(QStringLiteral("播放"));
    } else {
        m_player.play();
        m_playing = true;
        m_playBtn->setText(QStringLiteral("暂停"));
    }
}

void ReplayWidget::onStopPlay()
{
    m_player.stop();
    m_playing = false;
    m_playBtn->setText(QStringLiteral("播放"));
    m_timeSlider->setValue(0);
    updatePlaybackUI();
}

void ReplayWidget::onSeek(int value)
{
    m_player.seekTo(value);
}

void ReplayWidget::onSpeedChanged(int index)
{
    Q_UNUSED(index);
    double v = 0.5;
    switch (m_speedCombo->currentData().toInt()) {
    case 1: v = 0.5; break;
    case 2: v = 1.0; break;
    case 4: v = 2.0; break;
    case 8: v = 4.0; break;
    }
    m_player.setSpeed(v);
}

void ReplayWidget::applyReplaySourceFilter()
{
    QSet<int> sources;
    if (m_rpRedisCb->isChecked()) sources.insert(Protocol_Redis);
    if (m_rpDdsCb->isChecked()) sources.insert(Protocol_DDS);
    if (m_rpHttpCb->isChecked()) sources.insert(Protocol_HTTP);
    m_player.setEnabledSources(sources);
}

void ReplayWidget::onReplaySourcesChanged()
{
    applyReplaySourceFilter();
}

void ReplayWidget::onRecordStatus(const QString &msg)
{
    // 状态标签只显示简洁状态：录制中 / 开始录制
    if (m_recorder.isRecording()) {
        m_recStatusLabel->setText(QStringLiteral("状态: 录制中"));
    } else {
        m_recStatusLabel->setText(QStringLiteral("状态: 开始录制"));
    }
    Q_UNUSED(msg);
}

void ReplayWidget::onReplayStatus(const QString &msg)
{
    m_playStatusLabel->setText(QStringLiteral("状态: ") + msg);
}

void ReplayWidget::onPositionChanged(qint64 ms, int injected)
{
    if (m_playing) {
        m_timeSlider->setValue((int)ms);
    }
    m_lastInjected = injected;
    updatePlaybackUI();
}

void ReplayWidget::onPlaybackFinished()
{
    m_playing = false;
    m_playBtn->setText(QStringLiteral("播放"));
}

void ReplayWidget::updatePlaybackUI()
{
    const QString cur = QTime(0, 0).addMSecs(m_player.positionMs()).toString(QStringLiteral("mm:ss"));
    const QString total = QTime(0, 0).addMSecs(m_player.durationMs()).toString(QStringLiteral("mm:ss"));
    m_timeLabel->setText(QStringLiteral("%1 / %2  | 已注入 %3 条")
                         .arg(cur, total).arg(m_lastInjected));
}
