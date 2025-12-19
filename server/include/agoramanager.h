#ifndef AGORAMANAGER_H
#define AGORAMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QByteArray>

// Agora SDK integration
// Note: This requires Agora C++ SDK to be installed
// Download from: https://docs.agora.io/cn/Video/downloads

struct AgoraConfig {
    QString appId;
    QString appCertificate;
    int tokenExpirationSeconds;
    bool enableAudio;
    bool enableVideo;
};

struct ChannelInfo {
    QString channelName;
    QString channelId;
    QString hostUserId;
    QString guestUserId;
    QDateTime createdAt;
    bool isActive;
};

class AgoraManager : public QObject
{
    Q_OBJECT

public:
    explicit AgoraManager(QObject *parent = nullptr);
    ~AgoraManager();

    // Initialization
    bool initialize(const AgoraConfig &config);
    bool isInitialized() const { return m_initialized; }
    
    // Token generation (server-side)
    QString generateRtcToken(const QString &channelName, const QString &userId, int expirationSeconds = 3600);
    QString generateRtmToken(const QString &userId, int expirationSeconds = 3600);
    
    // Channel management
    QString createChannel(const QString &hostUserId, const QString &guestUserId);
    bool joinChannel(const QString &channelId, const QString &userId);
    bool leaveChannel(const QString &channelId, const QString &userId);
    bool closeChannel(const QString &channelId);
    ChannelInfo* getChannelInfo(const QString &channelId);
    QList<QString> getActiveChannels() const;
    
    // Audio file management
    bool saveAudioPCM(const QString &userId, const QByteArray &pcmData, const QString &filename);
    QByteArray loadAudioPCM(const QString &filename);
    
    // Audio data forwarding (for custom audio transmission over TCP)
    void forwardAudioData(const QString &channelId, const QString &fromUser, const QByteArray &audioData);

private:
    QString generateChannelId();
    QString calculateToken(const QString &appId, const QString &appCertificate,
                          const QString &channelName, const QString &userId, int expiration);
    
    bool m_initialized;
    AgoraConfig m_config;
    QMap<QString, ChannelInfo*> m_channels;  // channelId -> ChannelInfo
    QString m_audioStoragePath;

signals:
    void channelCreated(const QString &channelId, const QString &hostUserId, const QString &guestUserId);
    void userJoinedChannel(const QString &channelId, const QString &userId);
    void userLeftChannel(const QString &channelId, const QString &userId);
    void channelClosed(const QString &channelId);
    void audioDataReceived(const QString &channelId, const QString &userId, const QByteArray &data);
};

#endif // AGORAMANAGER_H
