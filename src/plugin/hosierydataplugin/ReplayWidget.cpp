/**
 * @file ReplayWidget.cpp
 * @brief 历史数据回放界面实现
 * @date 2026-11-23
 */

#include "ReplayWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QNetworkInterface>
#include <QSettings>
#include <QAbstractButton>
#include <QJsonObject>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTime>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>

/** @brief 文件大小格式化：<1MB 显示 KB，<1GB 显示 MB，否则 GB */
static QString formatFileSize(qint64 bytes)
{
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;
    if (bytes < MB) {
        return QStringLiteral("%1 KB").arg(bytes / KB, 0, 'f', 1);
    } else if (bytes < GB) {
        return QStringLiteral("%1 MB").arg(bytes / MB, 0, 'f', 1);
    }
    return QStringLiteral("%1 GB").arg(bytes / GB, 0, 'f', 2);
}

ReplayWidget::ReplayWidget(QWidget *parent)
    : QWidget(parent)
{
    qRegisterMetaType<ForwardPayload>("ForwardPayload");
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
    m_srcDdsCb->setChecked(false);     // 系统实际数据只有 Redis，DDS/HTTP 为预留
    m_srcRedisCb->setEnabled(false);
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

    // ========== 转发设置区 ==========
    QGroupBox *fwdGroup = new QGroupBox(QStringLiteral("转发设置（回放时发送）"), this);
    QVBoxLayout *fwd = new QVBoxLayout(fwdGroup);

    // 主单选：关闭/DDS/UDP/TCP
    QHBoxLayout *fwdModes = new QHBoxLayout();
    fwdModes->addWidget(new QLabel(QStringLiteral("方式:"), this));
    m_forwardGroup = new QButtonGroup(this);
    QRadioButton *rbOff = new QRadioButton(QStringLiteral("关闭"), this);
    QRadioButton *rbDds = new QRadioButton(QStringLiteral("DDS"), this);
    QRadioButton *rbUdp = new QRadioButton(QStringLiteral("UDP"), this);
    QRadioButton *rbTcp = new QRadioButton(QStringLiteral("TCP"), this);
    m_forwardGroup->addButton(rbOff, (int)ForwardManager::Off);
    m_forwardGroup->addButton(rbDds, (int)ForwardManager::Dds);
    m_forwardGroup->addButton(rbUdp, (int)ForwardManager::Udp);
    m_forwardGroup->addButton(rbTcp, (int)ForwardManager::Tcp);
    rbOff->setChecked(true);
    fwdModes->addWidget(rbOff);
    fwdModes->addWidget(rbDds);
    fwdModes->addWidget(rbUdp);
    fwdModes->addWidget(rbTcp);
    fwdModes->addStretch();
    fwd->addLayout(fwdModes);

    // 数据格式单选（JSON / 二进制；仅 UDP/TCP 有效，DDS 固定 JSON）
    QHBoxLayout *fmtRow = new QHBoxLayout();
    fmtRow->addWidget(new QLabel(QStringLiteral("数据格式:"), this));
    m_formatGroup = new QButtonGroup(this);
    QRadioButton *rbJson = new QRadioButton(QStringLiteral("JSON"), this);
    QRadioButton *rbBinary = new QRadioButton(QStringLiteral("二进制"), this);
    m_formatGroup->addButton(rbJson, (int)ForwardManager::Json);
    m_formatGroup->addButton(rbBinary, (int)ForwardManager::Binary);
    rbJson->setChecked(true);
    fmtRow->addWidget(rbJson);
    fmtRow->addWidget(rbBinary);
    // 字节序（仅二进制模式有效）
    fmtRow->addWidget(new QLabel(QStringLiteral("  字节序:"), this));
    m_byteOrderGroup = new QButtonGroup(this);
    QRadioButton *rbBig = new QRadioButton(QStringLiteral("大端"), this);
    QRadioButton *rbLittle = new QRadioButton(QStringLiteral("小端"), this);
    m_byteOrderGroup->addButton(rbBig, 1);
    m_byteOrderGroup->addButton(rbLittle, 0);
    rbBig->setChecked(true);
    fmtRow->addWidget(rbBig);
    fmtRow->addWidget(rbLittle);
    fmtRow->addStretch();
    fwd->addLayout(fmtRow);

    // 配置面板栈
    m_forwardStack = new QStackedWidget(this);
    QWidget *emptyPage = new QWidget(m_forwardStack);
    m_forwardStack->addWidget(emptyPage);   // Off

    // --- DDS 面板 ---
    QWidget *ddsPage = new QWidget(m_forwardStack);
    QFormLayout *ddsLayout = new QFormLayout(ddsPage);
    m_ddsDomainEdit = new QLineEdit(QStringLiteral("0"), ddsPage);
    m_ddsBindEdit = new QLineEdit(QStringLiteral("127.0.0.1"), ddsPage);
    m_ddsPlatformTopicEdit = new QLineEdit(QStringLiteral("cpss/platform"), ddsPage);
    m_ddsEventTopicEdit = new QLineEdit(QStringLiteral("cpss/event"), ddsPage);
    ddsLayout->addRow(QStringLiteral("Domain:"), m_ddsDomainEdit);
    ddsLayout->addRow(QStringLiteral("绑定IP:"), m_ddsBindEdit);
    ddsLayout->addRow(QStringLiteral("平台Topic:"), m_ddsPlatformTopicEdit);
    ddsLayout->addRow(QStringLiteral("事件Topic:"), m_ddsEventTopicEdit);
    m_forwardStack->addWidget(ddsPage);

    // --- UDP 面板（含单播/组播子栈）---
    QWidget *udpPage = new QWidget(m_forwardStack);
    QVBoxLayout *udpLayout = new QVBoxLayout(udpPage);
    QHBoxLayout *udpSub = new QHBoxLayout();
    udpSub->addWidget(new QLabel(QStringLiteral("子模式:"), udpPage));
    m_udpSubGroup = new QButtonGroup(udpPage);
    QRadioButton *rbUni = new QRadioButton(QStringLiteral("单播"), udpPage);
    QRadioButton *rbMcast = new QRadioButton(QStringLiteral("组播"), udpPage);
    m_udpSubGroup->addButton(rbUni, 0);
    m_udpSubGroup->addButton(rbMcast, 1);
    rbUni->setChecked(true);
    udpSub->addWidget(rbUni);
    udpSub->addWidget(rbMcast);
    udpSub->addWidget(new QLabel(QStringLiteral("  端口:"), udpPage));
    m_udpPortEdit = new QLineEdit(QStringLiteral("12345"), udpPage);
    m_udpPortEdit->setMaximumWidth(70);
    udpSub->addWidget(m_udpPortEdit);
    udpSub->addStretch();
    udpLayout->addLayout(udpSub);

    m_udpStack = new QStackedWidget(udpPage);
    // 单播页
    QWidget *uniPage = new QWidget(m_udpStack);
    QFormLayout *uniLayout = new QFormLayout(uniPage);
    m_udpTargetIpEdit = new QLineEdit(QStringLiteral("127.0.0.1"), uniPage);
    uniLayout->addRow(QStringLiteral("目标IP:"), m_udpTargetIpEdit);
    m_udpStack->addWidget(uniPage);
    // 组播页
    QWidget *mcastPage = new QWidget(m_udpStack);
    QFormLayout *mcastLayout = new QFormLayout(mcastPage);
    m_udpMcastAddrEdit = new QLineEdit(QStringLiteral("239.255.0.1"), mcastPage);
    m_udpTtlEdit = new QLineEdit(QStringLiteral("2"), mcastPage);
    m_udpIfCombo = new QComboBox(mcastPage);
    m_udpIfCombo->addItem(QStringLiteral("自动"), QString());
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp))
            m_udpIfCombo->addItem(iface.humanReadableName(), iface.name());
    }
    m_udpLoopbackCb = new QCheckBox(QStringLiteral("本机回环"), mcastPage);
    m_udpLoopbackCb->setChecked(true);
    mcastLayout->addRow(QStringLiteral("组播地址:"), m_udpMcastAddrEdit);
    mcastLayout->addRow(QStringLiteral("TTL:"), m_udpTtlEdit);
    mcastLayout->addRow(QStringLiteral("本地网卡:"), m_udpIfCombo);
    mcastLayout->addRow(QString(), m_udpLoopbackCb);
    m_udpStack->addWidget(mcastPage);
    udpLayout->addWidget(m_udpStack);
    m_forwardStack->addWidget(udpPage);

    // --- TCP 面板 ---
    QWidget *tcpPage = new QWidget(m_forwardStack);
    QFormLayout *tcpLayout = new QFormLayout(tcpPage);
    m_tcpIpEdit = new QLineEdit(QStringLiteral("127.0.0.1"), tcpPage);
    m_tcpPortEdit = new QLineEdit(QStringLiteral("12346"), tcpPage);
    m_tcpReconnectEdit = new QLineEdit(QStringLiteral("3000"), tcpPage);
    tcpLayout->addRow(QStringLiteral("目标IP:"), m_tcpIpEdit);
    tcpLayout->addRow(QStringLiteral("端口:"), m_tcpPortEdit);
    tcpLayout->addRow(QStringLiteral("重连间隔(ms):"), m_tcpReconnectEdit);
    m_forwardStack->addWidget(tcpPage);

    fwd->addWidget(m_forwardStack);
    m_forwardStatusLabel = new QLabel(QStringLiteral("状态: 转发关闭"), this);
    fwd->addWidget(m_forwardStatusLabel);
    main->addWidget(fwdGroup);

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
    connect(&m_player, &ReplayPlayer::forwardData, this, &ReplayWidget::onForwardData);

    // 转发模式切换
    connect(m_forwardGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &ReplayWidget::onForwardModeChanged);
    connect(m_udpSubGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &ReplayWidget::onUdpSubModeChanged);
    connect(m_formatGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &ReplayWidget::onForwardFormatChanged);
    connect(m_byteOrderGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &ReplayWidget::onByteOrderChanged);

    // 加载持久化配置
    loadForwardConfig();
    onForwardModeChanged(m_forwardGroup->checkedId());

    m_uiTimer.setInterval(200);
    connect(&m_uiTimer, &QTimer::timeout, this, &ReplayWidget::onRecordTimer);

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
    if (m_srcDdsCb->isChecked()) sources.insert(Protocol_FastDds);
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
        m_uiTimer.start();
        onRecordTimer();   // 立即显示一次状态
    }
}

void ReplayWidget::onStopRecord()
{
    const QString path = m_recorder.lastFilePath();
    const qint64 count = m_recorder.recordedCount();
    m_recorder.stop();
    m_uiTimer.stop();

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
        const QString size = formatFileSize(info.fileSize);
        QString src;
        for (int s : info.sources) {
            if (s == Protocol_Redis) src += "Redis ";
            else if (s == Protocol_FastDds) src += "DDS ";
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
    if (m_rpDdsCb->isChecked()) sources.insert(Protocol_FastDds);
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
    Q_UNUSED(injected);
    if (m_playing) {
        m_timeSlider->setValue((int)ms);
    }
    updatePlaybackUI();
}

void ReplayWidget::onPlaybackFinished()
{
    m_playing = false;
    m_playBtn->setText(QStringLiteral("播放"));
}

void ReplayWidget::onRecordTimer()
{
    if (!m_recorder.isRecording()) return;
    const QString path = m_recorder.lastFilePath();
    const QString fileName = QFileInfo(path).fileName();
    const qint64 bytes = QFileInfo(path).size();
    const QString size = formatFileSize(bytes);
    const qint64 elapsed = m_recorder.elapsedMs();
    const QString dur = QTime(0, 0).addMSecs(elapsed).toString(QStringLiteral("hh:mm:ss"));
    m_recStatusLabel->setText(QStringLiteral("状态: 录制中 | %1 | %2 | %3 条 | %4")
                              .arg(fileName, dur).arg(m_recorder.recordedCount()).arg(size));
}

void ReplayWidget::updatePlaybackUI()
{
    // 显示实际时钟时间：录制起始时刻 + 当前回放偏移
    const qint64 base = m_player.createTimeMs();
    const QString fmt = QStringLiteral("yyyy-MM-dd hh:mm:ss");
    QString cur, total;
    if (base > 0) {
        cur = QDateTime::fromMSecsSinceEpoch(base + m_player.positionMs()).toString(fmt);
        total = QDateTime::fromMSecsSinceEpoch(base + m_player.durationMs()).toString(fmt);
    } else {
        // 兼容旧文件（无起始时间）：回退到时长显示
        cur = QTime(0, 0).addMSecs(m_player.positionMs()).toString(QStringLiteral("mm:ss"));
        total = QTime(0, 0).addMSecs(m_player.durationMs()).toString(QStringLiteral("mm:ss"));
    }
    m_timeLabel->setText(QStringLiteral("%1 / %2").arg(cur, total));
}

// ==================== 转发设置 ====================

QJsonObject ReplayWidget::currentForwardConfig() const
{
    QJsonObject cfg;
    const int mode = m_forwardGroup->checkedId();

    // DDS 配置
    cfg["domain"] = m_ddsDomainEdit->text().toInt();
    cfg["bindAddress"] = m_ddsBindEdit->text();
    cfg["platformTopic"] = m_ddsPlatformTopicEdit->text();
    cfg["eventTopic"] = m_ddsEventTopicEdit->text();

    // UDP 配置
    cfg["subMode"] = (m_udpSubGroup->checkedId() == 1) ? "Multicast" : "Unicast";
    cfg["targetIp"] = m_udpTargetIpEdit->text();
    cfg["multicastAddr"] = m_udpMcastAddrEdit->text();
    cfg["port"] = m_udpPortEdit->text().toInt();
    cfg["ttl"] = m_udpTtlEdit->text().toInt();
    cfg["localInterface"] = m_udpIfCombo->currentData().toString();
    cfg["loopback"] = m_udpLoopbackCb->isChecked();

    // TCP 配置
    cfg["targetIpTcp"] = m_tcpIpEdit->text();
    cfg["tcpPort"] = m_tcpPortEdit->text().toInt();
    cfg["reconnectMs"] = m_tcpReconnectEdit->text().toInt();

    Q_UNUSED(mode);
    return cfg;
}

void ReplayWidget::loadForwardConfig()
{
    const QString ini = QCoreApplication::applicationDirPath() + "/cpss.ini";
    QSettings settings(ini, QSettings::IniFormat);
    const int mode = settings.value("ReplayForward/Mode", 0).toInt();
    QAbstractButton *btn = m_forwardGroup->button(mode);
    if (btn) btn->setChecked(true);
    const int format = settings.value("ReplayForward/Format", 0).toInt();
    QAbstractButton *fmtBtn = m_formatGroup->button(format);
    if (fmtBtn) fmtBtn->setChecked(true);
    const int byteOrder = settings.value("ReplayForward/ByteOrder", 1).toInt();
    QAbstractButton *boBtn = m_byteOrderGroup->button(byteOrder);
    if (boBtn) boBtn->setChecked(true);
    m_player.setBigEndian(m_byteOrderGroup->checkedId() == 1);

    m_ddsDomainEdit->setText(settings.value("ReplayForward.Dds/domain", "0").toString());
    m_ddsBindEdit->setText(settings.value("ReplayForward.Dds/bindAddress", "127.0.0.1").toString());
    m_ddsPlatformTopicEdit->setText(settings.value("ReplayForward.Dds/platformTopic", "cpss/platform").toString());
    m_ddsEventTopicEdit->setText(settings.value("ReplayForward.Dds/eventTopic", "cpss/event").toString());

    const QString sub = settings.value("ReplayForward.Udp/subMode", "Unicast").toString();
    QAbstractButton *subBtn = m_udpSubGroup->button(sub == "Multicast" ? 1 : 0);
    if (subBtn) subBtn->setChecked(true);
    m_udpTargetIpEdit->setText(settings.value("ReplayForward.Udp/targetIp", "127.0.0.1").toString());
    m_udpMcastAddrEdit->setText(settings.value("ReplayForward.Udp/multicastAddr", "239.255.0.1").toString());
    m_udpPortEdit->setText(settings.value("ReplayForward.Udp/port", "12345").toString());
    m_udpTtlEdit->setText(settings.value("ReplayForward.Udp/ttl", "2").toString());
    const QString ifName = settings.value("ReplayForward.Udp/localInterface", "").toString();
    const int ifIdx = m_udpIfCombo->findData(ifName);
    if (ifIdx >= 0) m_udpIfCombo->setCurrentIndex(ifIdx);
    m_udpLoopbackCb->setChecked(settings.value("ReplayForward.Udp/loopback", true).toBool());

    m_tcpIpEdit->setText(settings.value("ReplayForward.Tcp/targetIp", "127.0.0.1").toString());
    m_tcpPortEdit->setText(settings.value("ReplayForward.Tcp/port", "12346").toString());
    m_tcpReconnectEdit->setText(settings.value("ReplayForward.Tcp/reconnectMs", "3000").toString());
}

void ReplayWidget::saveForwardConfig()
{
    const QString ini = QCoreApplication::applicationDirPath() + "/cpss.ini";
    QSettings settings(ini, QSettings::IniFormat);
    settings.setValue("ReplayForward/Mode", m_forwardGroup->checkedId());
    settings.setValue("ReplayForward/Format", m_formatGroup->checkedId());
    settings.setValue("ReplayForward/ByteOrder", m_byteOrderGroup->checkedId());

    settings.setValue("ReplayForward.Dds/domain", m_ddsDomainEdit->text());
    settings.setValue("ReplayForward.Dds/bindAddress", m_ddsBindEdit->text());
    settings.setValue("ReplayForward.Dds/platformTopic", m_ddsPlatformTopicEdit->text());
    settings.setValue("ReplayForward.Dds/eventTopic", m_ddsEventTopicEdit->text());

    settings.setValue("ReplayForward.Udp/subMode",
                      (m_udpSubGroup->checkedId() == 1) ? "Multicast" : "Unicast");
    settings.setValue("ReplayForward.Udp/targetIp", m_udpTargetIpEdit->text());
    settings.setValue("ReplayForward.Udp/multicastAddr", m_udpMcastAddrEdit->text());
    settings.setValue("ReplayForward.Udp/port", m_udpPortEdit->text());
    settings.setValue("ReplayForward.Udp/ttl", m_udpTtlEdit->text());
    settings.setValue("ReplayForward.Udp/localInterface", m_udpIfCombo->currentData().toString());
    settings.setValue("ReplayForward.Udp/loopback", m_udpLoopbackCb->isChecked());

    settings.setValue("ReplayForward.Tcp/targetIp", m_tcpIpEdit->text());
    settings.setValue("ReplayForward.Tcp/port", m_tcpPortEdit->text());
    settings.setValue("ReplayForward.Tcp/reconnectMs", m_tcpReconnectEdit->text());
    settings.sync();
}

void ReplayWidget::onForwardModeChanged(int id)
{
    m_forwardStack->setCurrentIndex(qMax(0, id));
    // 关闭当前转发器，按新模式打开
    QJsonObject cfg = currentForwardConfig();
    // TCP 目标 IP 用专用字段
    cfg["targetIp"] = m_tcpIpEdit->text();
    cfg["port"] = (id == (int)ForwardManager::Tcp) ? m_tcpPortEdit->text().toInt()
                                                    : m_udpPortEdit->text().toInt();

    ForwardManager::Format fmt = (m_formatGroup->checkedId() == (int)ForwardManager::Binary)
                                 ? ForwardManager::Binary : ForwardManager::Json;
    bool ok = m_forwardManager.setMode((ForwardManager::Mode)id, cfg, fmt);
    if (id == (int)ForwardManager::Off) {
        m_forwardStatusLabel->setText(QStringLiteral("状态: 转发关闭"));
    } else if (ok) {
        m_forwardStatusLabel->setText(QStringLiteral("状态: 转发就绪 (%1)")
                                      .arg(id == (int)ForwardManager::Dds ? "DDS" :
                                           id == (int)ForwardManager::Udp ? "UDP" : "TCP"));
    } else {
        m_forwardStatusLabel->setText(QStringLiteral("状态: 转发启动失败 - ") + m_forwardManager.lastError());
    }
    saveForwardConfig();
}

void ReplayWidget::onUdpSubModeChanged(int id)
{
    m_udpStack->setCurrentIndex(qMax(0, id));
    // 如果当前是 UDP 模式，重新打开以应用子模式
    if (m_forwardGroup->checkedId() == (int)ForwardManager::Udp) {
        onForwardModeChanged((int)ForwardManager::Udp);
    }
}

void ReplayWidget::onForwardData(const ForwardPayload &payload)
{
    if (!m_forwardManager.isActive()) return;
    m_forwardManager.send(payload.json, payload.binary);
}

void ReplayWidget::onForwardFormatChanged(int id)
{
    Q_UNUSED(id);
    // 格式变化：重开当前转发器以应用
    const int modeId = m_forwardGroup->checkedId();
    if (modeId != (int)ForwardManager::Off) {
        onForwardModeChanged(modeId);
    }
    saveForwardConfig();
}

void ReplayWidget::onByteOrderChanged(int id)
{
    m_player.setBigEndian(id == 1);   // 1=大端, 0=小端
    saveForwardConfig();
}
