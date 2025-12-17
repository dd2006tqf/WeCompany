#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariantMap>

struct UserProfile {
    QString userId;
    QString username;
    QString nickname;
    QString avatar;
    QString email;
    QString phone;
    QString status;  // online, offline, busy, away
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct FriendRelation {
    QString relationId;
    QString userId;
    QString friendId;
    QString friendNickname;
    QString friendAvatar;
    QString remark;  // Friend remark/alias
    QDateTime createdAt;
};

struct OfflineMessage {
    QString messageId;
    QString fromUserId;
    QString toUserId;
    QString content;
    QString messageType;  // text, image, file, etc.
    QDateTime sentAt;
    bool delivered;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Connection management
    bool connectToDatabase(const QString &host, int port, const QString &dbName,
                          const QString &user, const QString &password);
    void disconnectFromDatabase();
    bool isConnected() const;
    
    // Database initialization
    bool createTables();
    bool initializeDatabase();
    
    // User management
    bool createUser(const QString &userId, const QString &username, 
                   const QString &passwordHash, const QString &salt);
    bool updateUserProfile(const UserProfile &profile);
    UserProfile getUserProfile(const QString &userId);
    UserProfile getUserProfileByUsername(const QString &username);
    bool userExists(const QString &username);
    bool updateUserStatus(const QString &userId, const QString &status);
    
    // Friend management
    bool addFriendRelation(const QString &userId, const QString &friendId);
    bool removeFriendRelation(const QString &userId, const QString &friendId);
    bool areFriends(const QString &userId, const QString &friendId);
    QList<FriendRelation> getFriendList(const QString &userId, int offset = 0, int limit = 50);
    int getFriendCount(const QString &userId);
    bool updateFriendRemark(const QString &userId, const QString &friendId, const QString &remark);
    
    // Message management
    bool saveOfflineMessage(const OfflineMessage &message);
    QList<OfflineMessage> getOfflineMessages(const QString &userId, int limit = 100);
    bool markMessagesAsDelivered(const QString &userId);
    bool deleteOldMessages(int daysOld = 30);

private:
    bool executeQuery(QSqlQuery &query, const QString &errorContext);
    QString generateId();
    
    QSqlDatabase m_db;
    bool m_connected;

signals:
    void databaseError(const QString &error);
    void userCreated(const QString &userId);
    void friendAdded(const QString &userId, const QString &friendId);
};

#endif // DATABASEMANAGER_H
