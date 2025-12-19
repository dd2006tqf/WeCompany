#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QDebug>
#include "tcpserver.h"
#include "videocallserver.h"
#include "authmanager.h"
#include "databasemanager.h"
#include "agoramanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("WeCompany Server");
    app.setApplicationVersion("2.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("WeCompany Server - Enhanced with Authentication, Database, and Agora SDK");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Server port (default: 8888)",
                                  "port", "8888");
    parser.addOption(portOption);
    
    QCommandLineOption dbHostOption("db-host", "MySQL database host (default: localhost)", "host", "localhost");
    parser.addOption(dbHostOption);
    
    QCommandLineOption dbPortOption("db-port", "MySQL database port (default: 3306)", "port", "3306");
    parser.addOption(dbPortOption);
    
    QCommandLineOption dbNameOption("db-name", "Database name (default: wecompany)", "name", "wecompany");
    parser.addOption(dbNameOption);
    
    QCommandLineOption dbUserOption("db-user", "Database user (default: root)", "user", "root");
    parser.addOption(dbUserOption);
    
    QCommandLineOption dbPassOption("db-pass", "Database password", "password", "");
    parser.addOption(dbPassOption);
    
    QCommandLineOption agoraAppIdOption("agora-appid", "Agora App ID", "appid", "");
    parser.addOption(agoraAppIdOption);
    
    QCommandLineOption agoraCertOption("agora-cert", "Agora App Certificate", "cert", "");
    parser.addOption(agoraCertOption);

    parser.process(app);

    bool ok;
    quint16 port = parser.value(portOption).toUShort(&ok);
    if (!ok) {
        qCritical() << "Invalid port number";
        return 1;
    }

    // Create and start TCP server
    TcpServer tcpServer;
    if (!tcpServer.startServer(port)) {
        qCritical() << "Failed to start TCP server";
        return 1;
    }

    // Create authentication manager
    AuthManager authManager;
    qInfo() << "Authentication manager initialized";

    // Create and connect to database (optional)
    DatabaseManager dbManager;
    QString dbHost = parser.value(dbHostOption);
    int dbPort = parser.value(dbPortOption).toInt();
    QString dbName = parser.value(dbNameOption);
    QString dbUser = parser.value(dbUserOption);
    QString dbPass = parser.value(dbPassOption);
    
    bool dbEnabled = false;
    if (!dbHost.isEmpty() && !dbName.isEmpty()) {
        if (dbManager.connectToDatabase(dbHost, dbPort, dbName, dbUser, dbPass)) {
            if (dbManager.initializeDatabase()) {
                qInfo() << "Database initialized successfully";
                dbEnabled = true;
            } else {
                qWarning() << "Failed to initialize database, running without DB";
            }
        } else {
            qWarning() << "Failed to connect to database, running without DB";
        }
    } else {
        qInfo() << "Database not configured, running without DB persistence";
    }

    // Create Agora manager (optional)
    AgoraManager agoraManager;
    QString agoraAppId = parser.value(agoraAppIdOption);
    QString agoraCert = parser.value(agoraCertOption);
    
    bool agoraEnabled = false;
    if (!agoraAppId.isEmpty()) {
        AgoraConfig config;
        config.appId = agoraAppId;
        config.appCertificate = agoraCert;
        config.tokenExpirationSeconds = 3600;
        config.enableAudio = true;
        config.enableVideo = true;
        
        if (agoraManager.initialize(config)) {
            qInfo() << "Agora SDK initialized successfully";
            agoraEnabled = true;
        } else {
            qWarning() << "Failed to initialize Agora SDK";
        }
    } else {
        qInfo() << "Agora SDK not configured, using basic call relay";
    }

    // Create video call server
    VideoCallServer videoCallServer(&tcpServer);

    // Connect signals for logging
    QObject::connect(&tcpServer, &TcpServer::clientConnected, [&](const QString &userId) {
        qInfo() << "Client connected:" << userId;
        if (dbEnabled) {
            dbManager.updateUserStatus(userId, "online");
        }
    });

    QObject::connect(&tcpServer, &TcpServer::clientDisconnected, [&](const QString &userId) {
        qInfo() << "Client disconnected:" << userId;
        if (dbEnabled) {
            dbManager.updateUserStatus(userId, "offline");
        }
    });

    QObject::connect(&videoCallServer, &VideoCallServer::callInitiated, 
                     [&](const QString &callId, const QString &caller, const QString &callee) {
        qInfo() << "Call initiated:" << callId << "from" << caller << "to" << callee;
        
        // Create Agora channel if enabled
        if (agoraEnabled) {
            QString channelId = agoraManager.createChannel(caller, callee);
            qInfo() << "Agora channel created:" << channelId;
        }
    });

    QObject::connect(&videoCallServer, &VideoCallServer::callAccepted, [](const QString &callId) {
        qInfo() << "Call accepted:" << callId;
    });

    QObject::connect(&videoCallServer, &VideoCallServer::callEnded, [&](const QString &callId) {
        qInfo() << "Call ended:" << callId;
    });
    
    // Setup token cleanup timer
    QTimer tokenCleanupTimer;
    QObject::connect(&tokenCleanupTimer, &QTimer::timeout, [&]() {
        authManager.cleanupExpiredTokens();
        if (dbEnabled) {
            dbManager.deleteOldMessages(30);
        }
    });
    const int ONE_HOUR_MS = 60 * 60 * 1000;  // 1 hour in milliseconds
    tokenCleanupTimer.start(ONE_HOUR_MS);

    qInfo() << "========================================";
    qInfo() << "WeCompany Server v2.0 started";
    qInfo() << "Port:" << port;
    qInfo() << "Database:" << (dbEnabled ? "Enabled" : "Disabled");
    qInfo() << "Agora SDK:" << (agoraEnabled ? "Enabled" : "Disabled");
    qInfo() << "========================================";
    qInfo() << "Press Ctrl+C to stop the server";

    return app.exec();
}
