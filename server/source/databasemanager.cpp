#include "databasemanager.h"
#include <QDebug>
#include <QSqlRecord>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), m_connected(false)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnectFromDatabase();
}

bool DatabaseManager::connectToDatabase(const QString &host, int port, const QString &dbName,
                                       const QString &user, const QString &password)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    
    if (!m_db.open()) {
        QString error = "Failed to connect to database: " + m_db.lastError().text();
        qCritical() << error;
        emit databaseError(error);
        return false;
    }
    
    m_connected = true;
    qInfo() << "Connected to database:" << dbName << "at" << host;
    return true;
}

void DatabaseManager::disconnectFromDatabase()
{
    if (m_connected) {
        m_db.close();
        m_connected = false;
        qInfo() << "Disconnected from database";
    }
}

bool DatabaseManager::isConnected() const
{
    return m_connected && m_db.isOpen();
}

bool DatabaseManager::executeQuery(QSqlQuery &query, const QString &errorContext)
{
    if (!query.exec()) {
        QString error = errorContext + ": " + query.lastError().text();
        qWarning() << error;
        emit databaseError(error);
        return false;
    }
    return true;
}

QString DatabaseManager::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);
    
    // Users table
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id VARCHAR(36) PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(64) NOT NULL,
            salt VARCHAR(32) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            INDEX idx_username (username)
        )
    )";
    
    query.prepare(createUsersTable);
    if (!executeQuery(query, "Create users table")) {
        return false;
    }
    
    // User profiles table
    QString createProfilesTable = R"(
        CREATE TABLE IF NOT EXISTS user_profiles (
            user_id VARCHAR(36) PRIMARY KEY,
            nickname VARCHAR(100),
            avatar VARCHAR(255),
            email VARCHAR(100),
            phone VARCHAR(20),
            status VARCHAR(20) DEFAULT 'offline',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
        )
    )";
    
    query.prepare(createProfilesTable);
    if (!executeQuery(query, "Create user_profiles table")) {
        return false;
    }
    
    // Friend relations table
    QString createFriendsTable = R"(
        CREATE TABLE IF NOT EXISTS friend_relations (
            relation_id VARCHAR(36) PRIMARY KEY,
            user_id VARCHAR(36) NOT NULL,
            friend_id VARCHAR(36) NOT NULL,
            remark VARCHAR(100),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            UNIQUE KEY unique_friendship (user_id, friend_id),
            INDEX idx_user_id (user_id),
            INDEX idx_friend_id (friend_id),
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
            FOREIGN KEY (friend_id) REFERENCES users(user_id) ON DELETE CASCADE
        )
    )";
    
    query.prepare(createFriendsTable);
    if (!executeQuery(query, "Create friend_relations table")) {
        return false;
    }
    
    // Offline messages table
    QString createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS offline_messages (
            message_id VARCHAR(36) PRIMARY KEY,
            from_user_id VARCHAR(36) NOT NULL,
            to_user_id VARCHAR(36) NOT NULL,
            content TEXT NOT NULL,
            message_type VARCHAR(20) DEFAULT 'text',
            sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            delivered BOOLEAN DEFAULT FALSE,
            INDEX idx_to_user (to_user_id, delivered),
            INDEX idx_sent_at (sent_at),
            FOREIGN KEY (from_user_id) REFERENCES users(user_id) ON DELETE CASCADE,
            FOREIGN KEY (to_user_id) REFERENCES users(user_id) ON DELETE CASCADE
        )
    )";
    
    query.prepare(createMessagesTable);
    if (!executeQuery(query, "Create offline_messages table")) {
        return false;
    }
    
    qInfo() << "Database tables created successfully";
    return true;
}

bool DatabaseManager::initializeDatabase()
{
    if (!isConnected()) {
        qWarning() << "Not connected to database";
        return false;
    }
    
    return createTables();
}

bool DatabaseManager::createUser(const QString &userId, const QString &username,
                                const QString &passwordHash, const QString &salt)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (user_id, username, password_hash, salt) VALUES (?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(username);
    query.addBindValue(passwordHash);
    query.addBindValue(salt);
    
    if (!executeQuery(query, "Create user")) {
        return false;
    }
    
    // Create default profile
    query.prepare("INSERT INTO user_profiles (user_id, nickname) VALUES (?, ?)");
    query.addBindValue(userId);
    query.addBindValue(username);  // Default nickname is username
    
    if (!executeQuery(query, "Create user profile")) {
        return false;
    }
    
    qInfo() << "User created:" << username;
    emit userCreated(userId);
    return true;
}

bool DatabaseManager::updateUserProfile(const UserProfile &profile)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE user_profiles 
        SET nickname = ?, avatar = ?, email = ?, phone = ?, status = ?
        WHERE user_id = ?
    )");
    query.addBindValue(profile.nickname);
    query.addBindValue(profile.avatar);
    query.addBindValue(profile.email);
    query.addBindValue(profile.phone);
    query.addBindValue(profile.status);
    query.addBindValue(profile.userId);
    
    return executeQuery(query, "Update user profile");
}

UserProfile DatabaseManager::getUserProfile(const QString &userId)
{
    UserProfile profile;
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT p.user_id, u.username, p.nickname, p.avatar, p.email, 
               p.phone, p.status, p.created_at, p.updated_at
        FROM user_profiles p
        JOIN users u ON p.user_id = u.user_id
        WHERE p.user_id = ?
    )");
    query.addBindValue(userId);
    
    if (executeQuery(query, "Get user profile") && query.next()) {
        profile.userId = query.value(0).toString();
        profile.username = query.value(1).toString();
        profile.nickname = query.value(2).toString();
        profile.avatar = query.value(3).toString();
        profile.email = query.value(4).toString();
        profile.phone = query.value(5).toString();
        profile.status = query.value(6).toString();
        profile.createdAt = query.value(7).toDateTime();
        profile.updatedAt = query.value(8).toDateTime();
    }
    
    return profile;
}

UserProfile DatabaseManager::getUserProfileByUsername(const QString &username)
{
    UserProfile profile;
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT p.user_id, u.username, p.nickname, p.avatar, p.email,
               p.phone, p.status, p.created_at, p.updated_at
        FROM user_profiles p
        JOIN users u ON p.user_id = u.user_id
        WHERE u.username = ?
    )");
    query.addBindValue(username);
    
    if (executeQuery(query, "Get user profile by username") && query.next()) {
        profile.userId = query.value(0).toString();
        profile.username = query.value(1).toString();
        profile.nickname = query.value(2).toString();
        profile.avatar = query.value(3).toString();
        profile.email = query.value(4).toString();
        profile.phone = query.value(5).toString();
        profile.status = query.value(6).toString();
        profile.createdAt = query.value(7).toDateTime();
        profile.updatedAt = query.value(8).toDateTime();
    }
    
    return profile;
}

bool DatabaseManager::userExists(const QString &username)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (executeQuery(query, "Check user exists") && query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}

bool DatabaseManager::updateUserStatus(const QString &userId, const QString &status)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE user_profiles SET status = ? WHERE user_id = ?");
    query.addBindValue(status);
    query.addBindValue(userId);
    
    return executeQuery(query, "Update user status");
}

bool DatabaseManager::addFriendRelation(const QString &userId, const QString &friendId)
{
    // Add bidirectional friendship
    QSqlQuery query(m_db);
    
    // Add user -> friend
    query.prepare("INSERT INTO friend_relations (relation_id, user_id, friend_id) VALUES (?, ?, ?)");
    query.addBindValue(generateId());
    query.addBindValue(userId);
    query.addBindValue(friendId);
    
    if (!executeQuery(query, "Add friend relation (1)")) {
        return false;
    }
    
    // Add friend -> user
    query.prepare("INSERT INTO friend_relations (relation_id, user_id, friend_id) VALUES (?, ?, ?)");
    query.addBindValue(generateId());
    query.addBindValue(friendId);
    query.addBindValue(userId);
    
    if (!executeQuery(query, "Add friend relation (2)")) {
        return false;
    }
    
    qInfo() << "Friend relation added:" << userId << "<->" << friendId;
    emit friendAdded(userId, friendId);
    return true;
}

bool DatabaseManager::removeFriendRelation(const QString &userId, const QString &friendId)
{
    QSqlQuery query(m_db);
    
    // Remove both directions
    query.prepare("DELETE FROM friend_relations WHERE (user_id = ? AND friend_id = ?) OR (user_id = ? AND friend_id = ?)");
    query.addBindValue(userId);
    query.addBindValue(friendId);
    query.addBindValue(friendId);
    query.addBindValue(userId);
    
    return executeQuery(query, "Remove friend relation");
}

bool DatabaseManager::areFriends(const QString &userId, const QString &friendId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM friend_relations WHERE user_id = ? AND friend_id = ?");
    query.addBindValue(userId);
    query.addBindValue(friendId);
    
    if (executeQuery(query, "Check friendship") && query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}

QList<FriendRelation> DatabaseManager::getFriendList(const QString &userId, int offset, int limit)
{
    QList<FriendRelation> friends;
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT fr.relation_id, fr.user_id, fr.friend_id, 
               p.nickname, p.avatar, fr.remark, fr.created_at
        FROM friend_relations fr
        JOIN user_profiles p ON fr.friend_id = p.user_id
        WHERE fr.user_id = ?
        ORDER BY fr.created_at DESC
        LIMIT ? OFFSET ?
    )");
    query.addBindValue(userId);
    query.addBindValue(limit);
    query.addBindValue(offset);
    
    if (executeQuery(query, "Get friend list")) {
        while (query.next()) {
            FriendRelation relation;
            relation.relationId = query.value(0).toString();
            relation.userId = query.value(1).toString();
            relation.friendId = query.value(2).toString();
            relation.friendNickname = query.value(3).toString();
            relation.friendAvatar = query.value(4).toString();
            relation.remark = query.value(5).toString();
            relation.createdAt = query.value(6).toDateTime();
            friends.append(relation);
        }
    }
    
    return friends;
}

int DatabaseManager::getFriendCount(const QString &userId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM friend_relations WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (executeQuery(query, "Get friend count") && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

bool DatabaseManager::updateFriendRemark(const QString &userId, const QString &friendId, const QString &remark)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE friend_relations SET remark = ? WHERE user_id = ? AND friend_id = ?");
    query.addBindValue(remark);
    query.addBindValue(userId);
    query.addBindValue(friendId);
    
    return executeQuery(query, "Update friend remark");
}

bool DatabaseManager::saveOfflineMessage(const OfflineMessage &message)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO offline_messages (message_id, from_user_id, to_user_id, content, message_type)
        VALUES (?, ?, ?, ?, ?)
    )");
    query.addBindValue(message.messageId);
    query.addBindValue(message.fromUserId);
    query.addBindValue(message.toUserId);
    query.addBindValue(message.content);
    query.addBindValue(message.messageType);
    
    return executeQuery(query, "Save offline message");
}

QList<OfflineMessage> DatabaseManager::getOfflineMessages(const QString &userId, int limit)
{
    QList<OfflineMessage> messages;
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT message_id, from_user_id, to_user_id, content, message_type, sent_at, delivered
        FROM offline_messages
        WHERE to_user_id = ? AND delivered = FALSE
        ORDER BY sent_at ASC
        LIMIT ?
    )");
    query.addBindValue(userId);
    query.addBindValue(limit);
    
    if (executeQuery(query, "Get offline messages")) {
        while (query.next()) {
            OfflineMessage msg;
            msg.messageId = query.value(0).toString();
            msg.fromUserId = query.value(1).toString();
            msg.toUserId = query.value(2).toString();
            msg.content = query.value(3).toString();
            msg.messageType = query.value(4).toString();
            msg.sentAt = query.value(5).toDateTime();
            msg.delivered = query.value(6).toBool();
            messages.append(msg);
        }
    }
    
    return messages;
}

bool DatabaseManager::markMessagesAsDelivered(const QString &userId)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE offline_messages SET delivered = TRUE WHERE to_user_id = ? AND delivered = FALSE");
    query.addBindValue(userId);
    
    return executeQuery(query, "Mark messages as delivered");
}

bool DatabaseManager::deleteOldMessages(int daysOld)
{
    QSqlQuery query(m_db);
    // MySQL-specific query using DATE_SUB
    // For other databases, use: WHERE sent_at < CURRENT_TIMESTAMP - INTERVAL ? DAY
    query.prepare("DELETE FROM offline_messages WHERE sent_at < DATE_SUB(NOW(), INTERVAL ? DAY) AND delivered = TRUE");
    query.addBindValue(daysOld);
    
    return executeQuery(query, "Delete old messages");
}
