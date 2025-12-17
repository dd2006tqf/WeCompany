#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QObject>

// Message types for communication protocol
enum MessageType {
    MSG_TEXT = 0,           // Text message
    MSG_FILE = 1,           // File transfer
    MSG_CALL_REQUEST = 2,   // Audio/Video call request
    MSG_CALL_ACCEPT = 3,    // Call accepted
    MSG_CALL_REJECT = 4,    // Call rejected
    MSG_CALL_END = 5,       // Call ended
    MSG_MEDIA_DATA = 6,     // Audio/Video media data
    MSG_HEARTBEAT = 7       // Heartbeat message
};

struct ClientInfo {
    QString userId;
    QTcpSocket* socket;
    QString ipAddress;
    quint16 port;
    bool isOnline;
};

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    bool startServer(quint16 port);
    void stopServer();
    bool sendMessage(const QString &userId, const QByteArray &data);
    void broadcastMessage(const QByteArray &data);
    QList<QString> getOnlineUsers() const;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientReadyRead();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError error);

private:
    void handleMessage(QTcpSocket *socket, const QByteArray &data);
    void registerClient(QTcpSocket *socket, const QString &userId);
    void removeClient(QTcpSocket *socket);

    QMap<QString, ClientInfo*> m_clients;  // userId -> ClientInfo
    QMap<QTcpSocket*, QString> m_socketToUserId;  // socket -> userId
    quint16 m_port;

signals:
    void clientConnected(const QString &userId);
    void clientDisconnected(const QString &userId);
    void messageReceived(const QString &userId, MessageType type, const QByteArray &data);
};

#endif // TCPSERVER_H
