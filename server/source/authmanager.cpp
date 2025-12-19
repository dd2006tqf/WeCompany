#include "authmanager.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QUuid>
#include <QCryptographicHash>
#include <QDateTime>

AuthManager::AuthManager(QObject *parent)
    : QObject(parent)
{
}

AuthManager::~AuthManager()
{
    qDeleteAll(m_users);
    qDeleteAll(m_tokens);
}

QString AuthManager::generateUserId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString AuthManager::generateSalt()
{
    QByteArray salt;
    for (int i = 0; i < 16; ++i) {
        salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return salt.toHex();
}

QString AuthManager::hashPassword(const QString &password, const QString &salt)
{
    QString combined = password + salt;
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

bool AuthManager::registerUser(const QString &username, const QString &password, QString &userId)
{
    if (username.isEmpty() || password.isEmpty()) {
        qWarning() << "Username or password is empty";
        return false;
    }
    
    if (userExists(username)) {
        qWarning() << "User already exists:" << username;
        return false;
    }
    
    userId = generateUserId();
    QString salt = generateSalt();
    QString passwordHash = hashPassword(password, salt);
    
    UserCredentials *cred = new UserCredentials;
    cred->userId = userId;
    cred->username = username;
    cred->passwordHash = passwordHash;
    cred->salt = salt;
    cred->createdAt = QDateTime::currentDateTime();
    cred->lastLogin = QDateTime();
    
    m_users[userId] = cred;
    m_usernameToId[username] = userId;
    
    qInfo() << "User registered:" << username << "ID:" << userId;
    emit userRegistered(userId, username);
    
    return true;
}

QString AuthManager::authenticateUser(const QString &username, const QString &password)
{
    if (!userExists(username)) {
        qWarning() << "User not found:" << username;
        return QString();
    }
    
    QString userId = m_usernameToId.value(username);
    UserCredentials *cred = m_users.value(userId);
    
    if (!cred) {
        qWarning() << "User credentials not found for:" << username;
        return QString();
    }
    
    QString passwordHash = hashPassword(password, cred->salt);
    
    if (passwordHash != cred->passwordHash) {
        qWarning() << "Invalid password for user:" << username;
        return QString();
    }
    
    // Update last login time
    cred->lastLogin = QDateTime::currentDateTime();
    
    // Generate new token
    QString token = generateToken(userId);
    
    qInfo() << "User authenticated:" << username << "Token issued";
    emit userAuthenticated(userId, token);
    
    return token;
}

QString AuthManager::generateToken(const QString &userId)
{
    // Revoke old token if exists
    QString oldToken = m_userTokens.value(userId);
    if (!oldToken.isEmpty()) {
        revokeToken(oldToken);
    }
    
    // Generate new token
    QString tokenData = userId + QDateTime::currentDateTime().toString(Qt::ISODate) + 
                       QString::number(QRandomGenerator::global()->generate());
    QString token = QCryptographicHash::hash(tokenData.toUtf8(), QCryptographicHash::Sha256).toHex();
    
    AuthToken *authToken = new AuthToken;
    authToken->token = token;
    authToken->userId = userId;
    authToken->issuedAt = QDateTime::currentDateTime();
    authToken->expiresAt = QDateTime::currentDateTime().addSecs(TOKEN_VALIDITY_HOURS * 3600);
    authToken->isValid = true;
    
    m_tokens[token] = authToken;
    m_userTokens[userId] = token;
    
    return token;
}

bool AuthManager::validateToken(const QString &token)
{
    AuthToken *authToken = m_tokens.value(token, nullptr);
    
    if (!authToken) {
        return false;
    }
    
    if (!authToken->isValid) {
        return false;
    }
    
    if (isTokenExpired(*authToken)) {
        authToken->isValid = false;
        return false;
    }
    
    return true;
}

QString AuthManager::getUserIdFromToken(const QString &token)
{
    AuthToken *authToken = m_tokens.value(token, nullptr);
    
    if (!authToken || !validateToken(token)) {
        return QString();
    }
    
    return authToken->userId;
}

bool AuthManager::revokeToken(const QString &token)
{
    AuthToken *authToken = m_tokens.value(token, nullptr);
    
    if (!authToken) {
        return false;
    }
    
    authToken->isValid = false;
    m_userTokens.remove(authToken->userId);
    
    qInfo() << "Token revoked for user:" << authToken->userId;
    emit tokenRevoked(authToken->userId);
    
    return true;
}

bool AuthManager::isTokenExpired(const AuthToken &token)
{
    return QDateTime::currentDateTime() > token.expiresAt;
}

void AuthManager::cleanupExpiredTokens()
{
    QList<QString> expiredTokens;
    
    for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it) {
        if (isTokenExpired(*it.value())) {
            it.value()->isValid = false;
            expiredTokens.append(it.key());
        }
    }
    
    for (const QString &token : expiredTokens) {
        AuthToken *authToken = m_tokens.value(token);
        if (authToken) {
            m_userTokens.remove(authToken->userId);
        }
    }
    
    if (!expiredTokens.isEmpty()) {
        qInfo() << "Cleaned up" << expiredTokens.size() << "expired tokens";
    }
}

bool AuthManager::userExists(const QString &username)
{
    return m_usernameToId.contains(username);
}

UserCredentials* AuthManager::getUserCredentials(const QString &userId)
{
    return m_users.value(userId, nullptr);
}
