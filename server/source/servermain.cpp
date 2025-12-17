#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "tcpserver.h"
#include "videocallserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("WeCompany Server");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("WeCompany Server - Audio/Video calling and TCP communication");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Server port (default: 8888)",
                                  "port", "8888");
    parser.addOption(portOption);

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

    // Create video call server
    VideoCallServer videoCallServer(&tcpServer);

    // Connect signals for logging
    QObject::connect(&tcpServer, &TcpServer::clientConnected, [](const QString &userId) {
        qInfo() << "Client connected:" << userId;
    });

    QObject::connect(&tcpServer, &TcpServer::clientDisconnected, [](const QString &userId) {
        qInfo() << "Client disconnected:" << userId;
    });

    QObject::connect(&videoCallServer, &VideoCallServer::callInitiated, 
                     [](const QString &callId, const QString &caller, const QString &callee) {
        qInfo() << "Call initiated:" << callId << "from" << caller << "to" << callee;
    });

    QObject::connect(&videoCallServer, &VideoCallServer::callAccepted, [](const QString &callId) {
        qInfo() << "Call accepted:" << callId;
    });

    QObject::connect(&videoCallServer, &VideoCallServer::callEnded, [](const QString &callId) {
        qInfo() << "Call ended:" << callId;
    });

    qInfo() << "WeCompany Server started on port" << port;
    qInfo() << "Press Ctrl+C to stop the server";

    return app.exec();
}
