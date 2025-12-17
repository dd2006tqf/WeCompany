#include "tcpserver.h"
#include <QDebug>
#include <QDataStream>
#include <QHostAddress>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent), m_port(0)
{
}

TcpServer::~TcpServer()
{
    stopServer();
    qDeleteAll(m_clients);
}

bool TcpServer::startServer(quint16 port)
{
    m_port = port;
    if (!listen(QHostAddress::Any, port)) {
        qWarning() << "Failed to start server on port" << port << ":" << errorString();
        return false;
    }
    
    qInfo() << "Server started successfully on port" << port;
    return true;
}

void TcpServer::stopServer()
{
    // Disconnect all clients
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value()->socket) {
            it.value()->socket->disconnectFromHost();
        }
    }
    
    close();
    qInfo() << "Server stopped";
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    
    if (socket->setSocketDescriptor(socketDescriptor)) {
        connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
                this, &TcpServer::onClientError);
        
        qInfo() << "New connection from" << socket->peerAddress().toString() 
                << ":" << socket->peerPort();
    } else {
        qWarning() << "Failed to set socket descriptor";
        socket->deleteLater();
    }
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    handleMessage(socket, data);
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString userId = m_socketToUserId.value(socket, QString());
    if (!userId.isEmpty()) {
        qInfo() << "Client disconnected:" << userId;
        emit clientDisconnected(userId);
    }
    
    removeClient(socket);
    socket->deleteLater();
}

void TcpServer::onClientError(QAbstractSocket::SocketError error)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    qWarning() << "Socket error:" << socket->errorString();
}

void TcpServer::handleMessage(QTcpSocket *socket, const QByteArray &data)
{
    if (data.isEmpty()) return;
    
    // Simple protocol: [MessageType(1 byte)][UserId length(2 bytes)][UserId][Data]
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_9);
    
    quint8 msgType;
    stream >> msgType;
    
    QString userId = m_socketToUserId.value(socket, QString());
    
    // If not registered, first message should contain userId
    if (userId.isEmpty()) {
        quint16 userIdLen;
        stream >> userIdLen;
        
        QByteArray userIdData(userIdLen, 0);
        stream.readRawData(userIdData.data(), userIdLen);
        userId = QString::fromUtf8(userIdData);
        
        registerClient(socket, userId);
        qInfo() << "Client registered:" << userId;
        emit clientConnected(userId);
    }
    
    // Extract remaining data
    QByteArray payload;
    while (!stream.atEnd()) {
        char byte;
        stream.readRawData(&byte, 1);
        payload.append(byte);
    }
    
    emit messageReceived(userId, static_cast<MessageType>(msgType), payload);
}

void TcpServer::registerClient(QTcpSocket *socket, const QString &userId)
{
    ClientInfo *info = new ClientInfo;
    info->userId = userId;
    info->socket = socket;
    info->ipAddress = socket->peerAddress().toString();
    info->port = socket->peerPort();
    info->isOnline = true;
    
    m_clients[userId] = info;
    m_socketToUserId[socket] = userId;
}

void TcpServer::removeClient(QTcpSocket *socket)
{
    QString userId = m_socketToUserId.value(socket, QString());
    if (!userId.isEmpty()) {
        ClientInfo *info = m_clients.value(userId, nullptr);
        if (info) {
            delete info;
            m_clients.remove(userId);
        }
        m_socketToUserId.remove(socket);
    }
}

bool TcpServer::sendMessage(const QString &userId, const QByteArray &data)
{
    ClientInfo *client = m_clients.value(userId, nullptr);
    if (!client || !client->socket || !client->isOnline) {
        qWarning() << "Client not found or offline:" << userId;
        return false;
    }
    
    qint64 written = client->socket->write(data);
    client->socket->flush();
    
    return written > 0;
}

void TcpServer::broadcastMessage(const QByteArray &data)
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value()->socket && it.value()->isOnline) {
            it.value()->socket->write(data);
            it.value()->socket->flush();
        }
    }
}

QList<QString> TcpServer::getOnlineUsers() const
{
    QList<QString> users;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value()->isOnline) {
            users.append(it.key());
        }
    }
    return users;
}
