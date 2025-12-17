#include "videocallserver.h"
#include "tcpserver.h"
#include <QDebug>
#include <QDateTime>
#include <QDataStream>
#include <QUuid>

VideoCallServer::VideoCallServer(TcpServer *tcpServer, QObject *parent)
    : QObject(parent), m_tcpServer(tcpServer), m_callIdCounter(0)
{
    connect(m_tcpServer, &TcpServer::messageReceived, 
            this, &VideoCallServer::onMessageReceived);
    connect(m_tcpServer, &TcpServer::clientDisconnected,
            this, &VideoCallServer::onClientDisconnected);
}

VideoCallServer::~VideoCallServer()
{
    qDeleteAll(m_callSessions);
}

QString VideoCallServer::generateCallId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool VideoCallServer::initiateCall(const QString &caller, const QString &callee, bool isVideo)
{
    // Check if either user is already in a call
    if (isUserInCall(caller)) {
        qWarning() << "Caller" << caller << "is already in a call";
        return false;
    }
    
    if (isUserInCall(callee)) {
        qWarning() << "Callee" << callee << "is already in a call";
        return false;
    }
    
    // Create new call session
    CallSession *session = new CallSession;
    session->callId = generateCallId();
    session->caller = caller;
    session->callee = callee;
    session->status = CALL_REQUESTING;
    session->isVideoCall = isVideo;
    session->startTime = QDateTime::currentMSecsSinceEpoch();
    session->endTime = 0;
    
    m_callSessions[session->callId] = session;
    m_userToCallId[caller] = session->callId;
    m_userToCallId[callee] = session->callId;
    
    // Send call request to callee
    sendCallRequest(callee, *session);
    
    qInfo() << "Call initiated:" << session->callId 
            << "from" << caller << "to" << callee
            << (isVideo ? "(video)" : "(audio)");
    
    emit callInitiated(session->callId, caller, callee);
    return true;
}

bool VideoCallServer::acceptCall(const QString &callId)
{
    CallSession *session = m_callSessions.value(callId, nullptr);
    if (!session) {
        qWarning() << "Call session not found:" << callId;
        return false;
    }
    
    if (session->status != CALL_REQUESTING) {
        qWarning() << "Call is not in requesting state:" << callId;
        return false;
    }
    
    session->status = CALL_ACTIVE;
    session->startTime = QDateTime::currentMSecsSinceEpoch();
    
    // Notify both parties
    sendCallResponse(session->caller, callId, true, QString());
    sendCallResponse(session->callee, callId, true, QString());
    
    qInfo() << "Call accepted:" << callId;
    emit callAccepted(callId);
    return true;
}

bool VideoCallServer::rejectCall(const QString &callId, const QString &reason)
{
    CallSession *session = m_callSessions.value(callId, nullptr);
    if (!session) {
        qWarning() << "Call session not found:" << callId;
        return false;
    }
    
    session->status = CALL_ENDED;
    session->endTime = QDateTime::currentMSecsSinceEpoch();
    
    // Notify both parties
    sendCallResponse(session->caller, callId, false, reason);
    sendCallResponse(session->callee, callId, false, reason);
    
    qInfo() << "Call rejected:" << callId << "reason:" << reason;
    emit callRejected(callId, reason);
    
    cleanupCall(callId);
    return true;
}

bool VideoCallServer::endCall(const QString &callId)
{
    CallSession *session = m_callSessions.value(callId, nullptr);
    if (!session) {
        qWarning() << "Call session not found:" << callId;
        return false;
    }
    
    session->status = CALL_ENDED;
    session->endTime = QDateTime::currentMSecsSinceEpoch();
    
    notifyCallEnd(*session);
    
    qInfo() << "Call ended:" << callId 
            << "duration:" << (session->endTime - session->startTime) << "ms";
    emit callEnded(callId);
    
    cleanupCall(callId);
    return true;
}

void VideoCallServer::relayMediaData(const QString &callId, const QString &fromUser, const QByteArray &mediaData)
{
    CallSession *session = m_callSessions.value(callId, nullptr);
    if (!session || session->status != CALL_ACTIVE) {
        return;
    }
    
    // Determine the recipient
    QString recipient = (fromUser == session->caller) ? session->callee : session->caller;
    
    // Prepare message
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_MEDIA_DATA);
    stream << callId;
    stream.writeRawData(mediaData.data(), mediaData.size());
    
    // Send to recipient
    m_tcpServer->sendMessage(recipient, message);
}

CallSession* VideoCallServer::getCallSession(const QString &callId)
{
    return m_callSessions.value(callId, nullptr);
}

QList<CallSession*> VideoCallServer::getActiveCalls() const
{
    QList<CallSession*> activeCalls;
    for (auto it = m_callSessions.begin(); it != m_callSessions.end(); ++it) {
        if (it.value()->status == CALL_ACTIVE || it.value()->status == CALL_REQUESTING) {
            activeCalls.append(it.value());
        }
    }
    return activeCalls;
}

bool VideoCallServer::isUserInCall(const QString &userId) const
{
    QString callId = m_userToCallId.value(userId, QString());
    if (callId.isEmpty()) {
        return false;
    }
    
    CallSession *session = m_callSessions.value(callId, nullptr);
    return session && (session->status == CALL_ACTIVE || session->status == CALL_REQUESTING);
}

void VideoCallServer::onMessageReceived(const QString &userId, int msgType, const QByteArray &data)
{
    MessageType type = static_cast<MessageType>(msgType);
    
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_9);
    
    switch (type) {
        case MSG_CALL_REQUEST: {
            QString callee;
            quint8 isVideo;
            stream >> callee >> isVideo;
            initiateCall(userId, callee, isVideo > 0);
            break;
        }
        
        case MSG_CALL_ACCEPT: {
            QString callId;
            stream >> callId;
            acceptCall(callId);
            break;
        }
        
        case MSG_CALL_REJECT: {
            QString callId, reason;
            stream >> callId >> reason;
            rejectCall(callId, reason);
            break;
        }
        
        case MSG_CALL_END: {
            QString callId;
            stream >> callId;
            endCall(callId);
            break;
        }
        
        case MSG_MEDIA_DATA: {
            QString callId = m_userToCallId.value(userId, QString());
            if (!callId.isEmpty()) {
                relayMediaData(callId, userId, data);
            }
            break;
        }
        
        default:
            // Not a call-related message, ignore
            break;
    }
}

void VideoCallServer::onClientDisconnected(const QString &userId)
{
    // If user was in a call, end it
    QString callId = m_userToCallId.value(userId, QString());
    if (!callId.isEmpty()) {
        endCall(callId);
    }
}

void VideoCallServer::sendCallRequest(const QString &callee, const CallSession &session)
{
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_CALL_REQUEST);
    stream << session.callId;
    stream << session.caller;
    stream << static_cast<quint8>(session.isVideoCall ? 1 : 0);
    
    m_tcpServer->sendMessage(callee, message);
}

void VideoCallServer::sendCallResponse(const QString &userId, const QString &callId, 
                                       bool accepted, const QString &reason)
{
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(accepted ? MSG_CALL_ACCEPT : MSG_CALL_REJECT);
    stream << callId;
    if (!accepted) {
        stream << reason;
    }
    
    m_tcpServer->sendMessage(userId, message);
}

void VideoCallServer::notifyCallEnd(const CallSession &session)
{
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_CALL_END);
    stream << session.callId;
    
    m_tcpServer->sendMessage(session.caller, message);
    m_tcpServer->sendMessage(session.callee, message);
}

void VideoCallServer::cleanupCall(const QString &callId)
{
    CallSession *session = m_callSessions.value(callId, nullptr);
    if (session) {
        m_userToCallId.remove(session->caller);
        m_userToCallId.remove(session->callee);
        m_callSessions.remove(callId);
        delete session;
    }
}
