#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUuid>

struct UserCredentials {
    QString userId;
    QString username;
    QString passwordHash;
    QString salt;
    QDateTime createdAt;
    QDateTime lastLogin;
};

struct AuthToken {
    QString token;
    QString userId;
    QDateTime issuedAt;
    QDateTime expiresAt;
    bool isValid;
};

class AuthManager : public QObject
{
    Q_OBJECT

public:
    explicit AuthManager(QObject *parent = nullptr);
    ~AuthManager();

    // User registration and authentication
    bool registerUser(const QString &username, const QString &password, QString &userId);
    QString authenticateUser(const QString &username, const QString &password);
    bool validateToken(const QString &token);
    QString getUserIdFromToken(const QString &token);
    
    // Token management
    QString generateToken(const QString &userId);
    bool revokeToken(const QString &token);
    void cleanupExpiredTokens();
    
    // Password management
    static QString hashPassword(const QString &password, const QString &salt);
    static QString generateSalt();
    
    // User management
    bool userExists(const QString &username);
    UserCredentials* getUserCredentials(const QString &userId);

private:
    QString generateUserId();
    bool isTokenExpired(const AuthToken &token);
    
    QMap<QString, UserCredentials*> m_users;        // userId -> UserCredentials
    QMap<QString, QString> m_usernameToId;          // username -> userId
    QMap<QString, AuthToken*> m_tokens;             // token -> AuthToken
    QMap<QString, QString> m_userTokens;            // userId -> current token
    
    static const int TOKEN_VALIDITY_HOURS = 24;

signals:
    void userRegistered(const QString &userId, const QString &username);
    void userAuthenticated(const QString &userId, const QString &token);
    void tokenRevoked(const QString &userId);
};

#endif // AUTHMANAGER_H
