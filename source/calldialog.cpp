#include "calldialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include "iconhelper.h"

// ========== OutgoingCallDialog ==========

OutgoingCallDialog::OutgoingCallDialog(const QString &calleeId, const QString &calleeName, 
                                     bool isVideo, QWidget *parent)
    : CBaseDlg(parent)
    , m_calleeId(calleeId)
    , m_calleeName(calleeName)
    , m_isVideo(isVideo)
    , m_callActive(false)
    , m_callDuration(0)
{
    createUI();
    
    m_durationTimer = new QTimer(this);
    connect(m_durationTimer, &QTimer::timeout, this, &OutgoingCallDialog::updateCallDuration);
}

OutgoingCallDialog::~OutgoingCallDialog()
{
    m_durationTimer->stop();
}

void OutgoingCallDialog::createUI()
{
    setFixedSize(400, 550);
    EnableMoveWindow(true);
    
    // Avatar
    m_labAvatar = new QLabel(this);
    m_labAvatar->setFixedSize(150, 150);
    m_labAvatar->setStyleSheet("border-image: url(:/qss/res/usricon.jpeg); border-radius: 75px;");
    m_labAvatar->setScaledContents(true);
    
    // Name
    m_labName = new QLabel(m_calleeName, this);
    m_labName->setAlignment(Qt::AlignCenter);
    m_labName->setStyleSheet("QLabel{font: bold 24px; color:#333333;}");
    
    // Status
    m_labStatus = new QLabel(m_isVideo ? tr("正在拨打视频通话...") : tr("正在拨打语音通话..."), this);
    m_labStatus->setAlignment(Qt::AlignCenter);
    m_labStatus->setStyleSheet("QLabel{font: 16px; color:#999999;}");
    
    // Duration (hidden initially)
    m_labDuration = new QLabel("00:00", this);
    m_labDuration->setAlignment(Qt::AlignCenter);
    m_labDuration->setStyleSheet("QLabel{font: 20px; color:#666666;}");
    m_labDuration->hide();
    
    // Hangup button
    m_btnHangup = new QPushButton(this);
    m_btnHangup->setFixedSize(80, 80);
    m_btnHangup->setStyleSheet(R"(
        QPushButton {
            background-color: #FF4444;
            border: none;
            border-radius: 40px;
            font: bold 16px;
            color: white;
        }
        QPushButton:hover {
            background-color: #FF2222;
        }
        QPushButton:pressed {
            background-color: #DD0000;
        }
    )");
    IconHelper::SetIcon(m_btnHangup, QChar(0xe60e), 40);
    connect(m_btnHangup, &QPushButton::clicked, this, &OutgoingCallDialog::onHangupClicked);
    
    // Layout
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addStretch();
    avatarLayout->addWidget(m_labAvatar);
    avatarLayout->addStretch();
    
    QHBoxLayout *hangupLayout = new QHBoxLayout();
    hangupLayout->addStretch();
    hangupLayout->addWidget(m_btnHangup);
    hangupLayout->addStretch();
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addSpacing(60);
    mainLayout->addLayout(avatarLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_labName);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_labStatus);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_labDuration);
    mainLayout->addStretch();
    mainLayout->addLayout(hangupLayout);
    mainLayout->addSpacing(60);
    
    centralWidget()->setLayout(mainLayout);
}

void OutgoingCallDialog::startCall()
{
    m_callActive = true;
    m_labStatus->setText(tr("通话中"));
    m_labDuration->show();
    m_durationTimer->start(1000);
}

void OutgoingCallDialog::onHangupClicked()
{
    if (m_callActive) {
        emit callEnded();
    } else {
        emit callCancelled();
    }
    accept();
}

void OutgoingCallDialog::updateCallDuration()
{
    m_callDuration++;
    int minutes = m_callDuration / 60;
    int seconds = m_callDuration % 60;
    m_labDuration->setText(QString("%1:%2")
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0')));
}

// ========== IncomingCallDialog ==========

IncomingCallDialog::IncomingCallDialog(const QString &callerId, const QString &callerName,
                                     bool isVideo, QWidget *parent)
    : CBaseDlg(parent)
    , m_callerId(callerId)
    , m_callerName(callerName)
    , m_isVideo(isVideo)
{
    createUI();
}

IncomingCallDialog::~IncomingCallDialog()
{
}

void IncomingCallDialog::createUI()
{
    setFixedSize(400, 550);
    EnableMoveWindow(true);
    
    // Avatar
    m_labAvatar = new QLabel(this);
    m_labAvatar->setFixedSize(150, 150);
    m_labAvatar->setStyleSheet("border-image: url(:/qss/res/usricon.jpeg); border-radius: 75px;");
    m_labAvatar->setScaledContents(true);
    
    // Name
    m_labName = new QLabel(m_callerName, this);
    m_labName->setAlignment(Qt::AlignCenter);
    m_labName->setStyleSheet("QLabel{font: bold 24px; color:#333333;}");
    
    // Status
    m_labStatus = new QLabel(m_isVideo ? tr("邀请你进行视频通话") : tr("邀请你进行语音通话"), this);
    m_labStatus->setAlignment(Qt::AlignCenter);
    m_labStatus->setStyleSheet("QLabel{font: 18px; color:#1485EE;}");
    
    // Accept button
    m_btnAccept = new QPushButton(this);
    m_btnAccept->setFixedSize(80, 80);
    m_btnAccept->setStyleSheet(R"(
        QPushButton {
            background-color: #00CC44;
            border: none;
            border-radius: 40px;
            font: bold 16px;
            color: white;
        }
        QPushButton:hover {
            background-color: #00AA33;
        }
        QPushButton:pressed {
            background-color: #008822;
        }
    )");
    IconHelper::SetIcon(m_btnAccept, QChar(0xe615), 40);
    connect(m_btnAccept, &QPushButton::clicked, this, &IncomingCallDialog::onAcceptClicked);
    
    // Reject button
    m_btnReject = new QPushButton(this);
    m_btnReject->setFixedSize(80, 80);
    m_btnReject->setStyleSheet(R"(
        QPushButton {
            background-color: #FF4444;
            border: none;
            border-radius: 40px;
            font: bold 16px;
            color: white;
        }
        QPushButton:hover {
            background-color: #FF2222;
        }
        QPushButton:pressed {
            background-color: #DD0000;
        }
    )");
    IconHelper::SetIcon(m_btnReject, QChar(0xe60e), 40);
    connect(m_btnReject, &QPushButton::clicked, this, &IncomingCallDialog::onRejectClicked);
    
    // Layout
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addStretch();
    avatarLayout->addWidget(m_labAvatar);
    avatarLayout->addStretch();
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_btnReject);
    buttonsLayout->addSpacing(60);
    buttonsLayout->addWidget(m_btnAccept);
    buttonsLayout->addStretch();
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addSpacing(60);
    mainLayout->addLayout(avatarLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_labName);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_labStatus);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addSpacing(60);
    
    centralWidget()->setLayout(mainLayout);
}

void IncomingCallDialog::onAcceptClicked()
{
    emit callAccepted();
    accept();
}

void IncomingCallDialog::onRejectClicked()
{
    emit callRejected();
    reject();
}

// ========== ActiveCallDialog ==========

ActiveCallDialog::ActiveCallDialog(const QString &peerId, const QString &peerName,
                                 bool isVideo, QWidget *parent)
    : CBaseDlg(parent)
    , m_peerId(peerId)
    , m_peerName(peerName)
    , m_isVideo(isVideo)
    , m_isMuted(false)
    , m_isVideoEnabled(isVideo)
    , m_callDuration(0)
{
    createUI();
    
    m_durationTimer = new QTimer(this);
    connect(m_durationTimer, &QTimer::timeout, this, &ActiveCallDialog::updateCallDuration);
    m_durationTimer->start(1000);
}

ActiveCallDialog::~ActiveCallDialog()
{
    m_durationTimer->stop();
}

void ActiveCallDialog::createUI()
{
    setFixedSize(600, 700);
    EnableMoveWindow(true);
    
    // Video preview (for video calls)
    m_labVideoPreview = new QLabel(this);
    m_labVideoPreview->setFixedSize(560, 400);
    m_labVideoPreview->setStyleSheet("QLabel{background-color: #000000; border-radius: 8px;}");
    if (!m_isVideo) {
        m_labVideoPreview->hide();
    }
    
    // Avatar (for audio calls or when video is off)
    m_labAvatar = new QLabel(this);
    m_labAvatar->setFixedSize(120, 120);
    m_labAvatar->setStyleSheet("border-image: url(:/qss/res/usricon.jpeg); border-radius: 60px;");
    if (m_isVideo) {
        m_labAvatar->hide();
    }
    
    // Name
    m_labName = new QLabel(m_peerName, this);
    m_labName->setAlignment(Qt::AlignCenter);
    m_labName->setStyleSheet("QLabel{font: bold 20px; color:#FFFFFF;}");
    
    // Duration
    m_labDuration = new QLabel("00:00", this);
    m_labDuration->setAlignment(Qt::AlignCenter);
    m_labDuration->setStyleSheet("QLabel{font: 18px; color:#CCCCCC;}");
    
    // Control buttons
    m_btnMute = new QPushButton(this);
    m_btnMute->setFixedSize(60, 60);
    m_btnMute->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.3);
            border: none;
            border-radius: 30px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.5);
        }
    )");
    IconHelper::SetIcon(m_btnMute, QChar(0xe641), 30);
    connect(m_btnMute, &QPushButton::clicked, this, &ActiveCallDialog::onMuteClicked);
    
    m_btnVideo = new QPushButton(this);
    m_btnVideo->setFixedSize(60, 60);
    m_btnVideo->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.3);
            border: none;
            border-radius: 30px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.5);
        }
    )");
    IconHelper::SetIcon(m_btnVideo, QChar(0xe647), 30);
    connect(m_btnVideo, &QPushButton::clicked, this, &ActiveCallDialog::onVideoClicked);
    if (!m_isVideo) {
        m_btnVideo->hide();
    }
    
    m_btnHangup = new QPushButton(this);
    m_btnHangup->setFixedSize(70, 70);
    m_btnHangup->setStyleSheet(R"(
        QPushButton {
            background-color: #FF4444;
            border: none;
            border-radius: 35px;
        }
        QPushButton:hover {
            background-color: #FF2222;
        }
    )");
    IconHelper::SetIcon(m_btnHangup, QChar(0xe60e), 35);
    connect(m_btnHangup, &QPushButton::clicked, this, &ActiveCallDialog::onHangupClicked);
    
    // Layout
    QHBoxLayout *videoLayout = new QHBoxLayout();
    videoLayout->addWidget(m_labVideoPreview);
    
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addStretch();
    avatarLayout->addWidget(m_labAvatar);
    avatarLayout->addStretch();
    
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_btnMute);
    controlsLayout->addSpacing(30);
    controlsLayout->addWidget(m_btnHangup);
    controlsLayout->addSpacing(30);
    if (m_isVideo) {
        controlsLayout->addWidget(m_btnVideo);
        controlsLayout->addStretch();
    } else {
        controlsLayout->addStretch();
    }
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addSpacing(20);
    if (m_isVideo) {
        mainLayout->addLayout(videoLayout);
    } else {
        mainLayout->addSpacing(80);
        mainLayout->addLayout(avatarLayout);
    }
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_labName);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_labDuration);
    mainLayout->addStretch();
    mainLayout->addLayout(controlsLayout);
    mainLayout->addSpacing(40);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    centralWidget()->setLayout(mainLayout);
    
    // Set dark background for video call
    if (m_isVideo) {
        centralWidget()->setStyleSheet("QWidget{background-color: #1A1A1A;}");
    }
}

void ActiveCallDialog::onHangupClicked()
{
    emit callEnded();
    accept();
}

void ActiveCallDialog::onMuteClicked()
{
    m_isMuted = !m_isMuted;
    IconHelper::SetIcon(m_btnMute, m_isMuted ? QChar(0xe640) : QChar(0xe641), 30);
    emit muteToggled(m_isMuted);
}

void ActiveCallDialog::onVideoClicked()
{
    m_isVideoEnabled = !m_isVideoEnabled;
    IconHelper::SetIcon(m_btnVideo, m_isVideoEnabled ? QChar(0xe647) : QChar(0xe646), 30);
    emit videoToggled(m_isVideoEnabled);
}

void ActiveCallDialog::updateCallDuration()
{
    m_callDuration++;
    int minutes = m_callDuration / 60;
    int seconds = m_callDuration % 60;
    m_labDuration->setText(QString("%1:%2")
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0')));
}
