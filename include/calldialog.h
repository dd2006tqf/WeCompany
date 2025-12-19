#ifndef CALLDIALOG_H
#define CALLDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include "basedlg.h"

// Outgoing call dialog
class OutgoingCallDialog : public CBaseDlg
{
    Q_OBJECT

public:
    explicit OutgoingCallDialog(const QString &calleeId, const QString &calleeName, 
                               bool isVideo, QWidget *parent = nullptr);
    ~OutgoingCallDialog();

signals:
    void callCancelled();
    void callEnded();

private slots:
    void onHangupClicked();
    void updateCallDuration();

private:
    void createUI();
    void startCall();

    QString m_calleeId;
    QString m_calleeName;
    bool m_isVideo;
    bool m_callActive;
    int m_callDuration;  // in seconds

    QLabel *m_labAvatar;
    QLabel *m_labName;
    QLabel *m_labStatus;
    QLabel *m_labDuration;
    QPushButton *m_btnHangup;
    QPushButton *m_btnMute;
    QPushButton *m_btnVideo;
    
    QTimer *m_durationTimer;
};

// Incoming call dialog
class IncomingCallDialog : public CBaseDlg
{
    Q_OBJECT

public:
    explicit IncomingCallDialog(const QString &callerId, const QString &callerName,
                               bool isVideo, QWidget *parent = nullptr);
    ~IncomingCallDialog();

signals:
    void callAccepted();
    void callRejected();

private slots:
    void onAcceptClicked();
    void onRejectClicked();

private:
    void createUI();

    QString m_callerId;
    QString m_callerName;
    bool m_isVideo;

    QLabel *m_labAvatar;
    QLabel *m_labName;
    QLabel *m_labStatus;
    QPushButton *m_btnAccept;
    QPushButton *m_btnReject;
};

// Active call dialog
class ActiveCallDialog : public CBaseDlg
{
    Q_OBJECT

public:
    explicit ActiveCallDialog(const QString &peerId, const QString &peerName,
                             bool isVideo, QWidget *parent = nullptr);
    ~ActiveCallDialog();

signals:
    void callEnded();
    void muteToggled(bool muted);
    void videoToggled(bool enabled);

private slots:
    void onHangupClicked();
    void onMuteClicked();
    void onVideoClicked();
    void updateCallDuration();

private:
    void createUI();

    QString m_peerId;
    QString m_peerName;
    bool m_isVideo;
    bool m_isMuted;
    bool m_isVideoEnabled;
    int m_callDuration;

    QLabel *m_labAvatar;
    QLabel *m_labName;
    QLabel *m_labDuration;
    QLabel *m_labVideoPreview;
    QPushButton *m_btnHangup;
    QPushButton *m_btnMute;
    QPushButton *m_btnVideo;
    
    QTimer *m_durationTimer;
};

#endif // CALLDIALOG_H
