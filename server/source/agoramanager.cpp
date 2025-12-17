#include "agoramanager.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUuid>
#include <QFile>
#include <QDir>
#include <QDataStream>

AgoraManager::AgoraManager(QObject *parent)
    : QObject(parent), m_initialized(false)
{
    m_audioStoragePath = "./audio_storage/";
    QDir dir;
    if (!dir.exists(m_audioStoragePath)) {
        dir.mkpath(m_audioStoragePath);
    }
}

AgoraManager::~AgoraManager()
{
    qDeleteAll(m_channels);
}

bool AgoraManager::initialize(const AgoraConfig &config)
{
    if (config.appId.isEmpty()) {
        qWarning() << "Agora App ID is empty";
        return false;
    }
    
    m_config = config;
    
    // In a real implementation, initialize Agora SDK here
    // For this implementation, we're providing the framework
    
    m_initialized = true;
    qInfo() << "Agora Manager initialized with App ID:" << config.appId;
    
    return true;
}

QString AgoraManager::generateChannelId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(16);
}

QString AgoraManager::calculateToken(const QString &appId, const QString &appCertificate,
                                    const QString &channelName, const QString &userId, int expiration)
{
    // Simplified token generation (mock implementation)
    // In production, use Agora's official token generation library
    // See: https://docs.agora.io/en/Video/token_server_cpp
    
    QString tokenData = appId + appCertificate + channelName + userId + 
                       QString::number(QDateTime::currentSecsSinceEpoch() + expiration);
    
    QByteArray hash = QCryptographicHash::hash(tokenData.toUtf8(), QCryptographicHash::Sha256);
    return "006" + hash.toHex();  // 006 prefix indicates token version
}

QString AgoraManager::generateRtcToken(const QString &channelName, const QString &userId, int expirationSeconds)
{
    if (!m_initialized) {
        qWarning() << "Agora Manager not initialized";
        return QString();
    }
    
    if (m_config.appCertificate.isEmpty()) {
        qWarning() << "App Certificate not configured, returning App ID only";
        return m_config.appId;
    }
    
    QString token = calculateToken(m_config.appId, m_config.appCertificate, 
                                   channelName, userId, expirationSeconds);
    
    qInfo() << "Generated RTC token for channel:" << channelName << "user:" << userId;
    return token;
}

QString AgoraManager::generateRtmToken(const QString &userId, int expirationSeconds)
{
    if (!m_initialized) {
        qWarning() << "Agora Manager not initialized";
        return QString();
    }
    
    if (m_config.appCertificate.isEmpty()) {
        qWarning() << "App Certificate not configured";
        return QString();
    }
    
    QString token = calculateToken(m_config.appId, m_config.appCertificate,
                                   "", userId, expirationSeconds);
    
    qInfo() << "Generated RTM token for user:" << userId;
    return token;
}

QString AgoraManager::createChannel(const QString &hostUserId, const QString &guestUserId)
{
    QString channelId = generateChannelId();
    QString channelName = "channel_" + channelId;
    
    ChannelInfo *info = new ChannelInfo;
    info->channelName = channelName;
    info->channelId = channelId;
    info->hostUserId = hostUserId;
    info->guestUserId = guestUserId;
    info->createdAt = QDateTime::currentDateTime();
    info->isActive = true;
    
    m_channels[channelId] = info;
    
    qInfo() << "Channel created:" << channelId << "host:" << hostUserId << "guest:" << guestUserId;
    emit channelCreated(channelId, hostUserId, guestUserId);
    
    return channelId;
}

bool AgoraManager::joinChannel(const QString &channelId, const QString &userId)
{
    ChannelInfo *info = m_channels.value(channelId, nullptr);
    if (!info) {
        qWarning() << "Channel not found:" << channelId;
        return false;
    }
    
    if (!info->isActive) {
        qWarning() << "Channel is not active:" << channelId;
        return false;
    }
    
    qInfo() << "User joined channel:" << userId << "channel:" << channelId;
    emit userJoinedChannel(channelId, userId);
    
    return true;
}

bool AgoraManager::leaveChannel(const QString &channelId, const QString &userId)
{
    ChannelInfo *info = m_channels.value(channelId, nullptr);
    if (!info) {
        qWarning() << "Channel not found:" << channelId;
        return false;
    }
    
    qInfo() << "User left channel:" << userId << "channel:" << channelId;
    emit userLeftChannel(channelId, userId);
    
    return true;
}

bool AgoraManager::closeChannel(const QString &channelId)
{
    ChannelInfo *info = m_channels.value(channelId, nullptr);
    if (!info) {
        qWarning() << "Channel not found:" << channelId;
        return false;
    }
    
    info->isActive = false;
    
    qInfo() << "Channel closed:" << channelId;
    emit channelClosed(channelId);
    
    return true;
}

ChannelInfo* AgoraManager::getChannelInfo(const QString &channelId)
{
    return m_channels.value(channelId, nullptr);
}

QList<QString> AgoraManager::getActiveChannels() const
{
    QList<QString> activeChannels;
    
    for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
        if (it.value()->isActive) {
            activeChannels.append(it.key());
        }
    }
    
    return activeChannels;
}

bool AgoraManager::saveAudioPCM(const QString &userId, const QByteArray &pcmData, const QString &filename)
{
    QString filepath = m_audioStoragePath + userId + "_" + filename;
    
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filepath;
        return false;
    }
    
    qint64 written = file.write(pcmData);
    file.close();
    
    if (written != pcmData.size()) {
        qWarning() << "Failed to write complete audio data";
        return false;
    }
    
    qInfo() << "Audio PCM saved:" << filepath << "size:" << pcmData.size() << "bytes";
    return true;
}

QByteArray AgoraManager::loadAudioPCM(const QString &filename)
{
    QString filepath = m_audioStoragePath + filename;
    
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filepath;
        return QByteArray();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    qInfo() << "Audio PCM loaded:" << filepath << "size:" << data.size() << "bytes";
    return data;
}

void AgoraManager::forwardAudioData(const QString &channelId, const QString &fromUser, const QByteArray &audioData)
{
    ChannelInfo *info = m_channels.value(channelId, nullptr);
    if (!info || !info->isActive) {
        qWarning() << "Cannot forward audio: invalid or inactive channel" << channelId;
        return;
    }
    
    // Determine recipient
    QString recipient = (fromUser == info->hostUserId) ? info->guestUserId : info->hostUserId;
    
    qDebug() << "Forwarding audio data in channel:" << channelId 
             << "from:" << fromUser << "to:" << recipient 
             << "size:" << audioData.size() << "bytes";
    
    emit audioDataReceived(channelId, recipient, audioData);
}
