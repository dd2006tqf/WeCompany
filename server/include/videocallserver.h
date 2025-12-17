#ifndef VIDEOCALLSERVER_H
#define VIDEOCALLSERVER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QByteArray>

enum CallStatus {
    CALL_IDLE = 0,
    CALL_REQUESTING = 1,
    CALL_RINGING = 2,
    CALL_ACTIVE = 3,
    CALL_ENDED = 4
};

struct CallSession {
    QString callId;
    QString caller;
    QString callee;
    CallStatus status;
    bool isVideoCall;
    qint64 startTime;
    qint64 endTime;
};

class TcpServer;

class VideoCallServer : public QObject
{
    Q_OBJECT

public:
    explicit VideoCallServer(TcpServer *tcpServer, QObject *parent = nullptr);
    ~VideoCallServer();

    // Call control methods
    bool initiateCall(const QString &caller, const QString &callee, bool isVideo);
    bool acceptCall(const QString &callId);
    bool rejectCall(const QString &callId, const QString &reason);
    bool endCall(const QString &callId);
    
    // Media relay methods
    void relayMediaData(const QString &callId, const QString &fromUser, const QByteArray &mediaData);
    
    // Query methods
    CallSession* getCallSession(const QString &callId);
    QList<CallSession*> getActiveCalls() const;
    bool isUserInCall(const QString &userId) const;

private slots:
    void onMessageReceived(const QString &userId, int msgType, const QByteArray &data);
    void onClientDisconnected(const QString &userId);

private:
    QString generateCallId();
    void sendCallRequest(const QString &callee, const CallSession &session);
    void sendCallResponse(const QString &userId, const QString &callId, bool accepted, const QString &reason);
    void notifyCallEnd(const CallSession &session);
    void cleanupCall(const QString &callId);

    TcpServer *m_tcpServer;
    QMap<QString, CallSession*> m_callSessions;  // callId -> CallSession
    QMap<QString, QString> m_userToCallId;       // userId -> callId (for active calls)
    int m_callIdCounter;

signals:
    void callInitiated(const QString &callId, const QString &caller, const QString &callee);
    void callAccepted(const QString &callId);
    void callRejected(const QString &callId, const QString &reason);
    void callEnded(const QString &callId);
};

#endif // VIDEOCALLSERVER_H
