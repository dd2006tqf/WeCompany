#include "serverlogindlg.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDataStream>
#include <QDebug>
#include "iconhelper.h"

ServerLoginDlg::ServerLoginDlg(QWidget *parent)
    : CBaseDlg(parent)
    , m_socket(nullptr)
    , m_serverHost("127.0.0.1")
    , m_serverPort(8888)
    , m_isRegistering(false)
{
    createUI();
    
    // Initialize socket
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &ServerLoginDlg::onServerConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ServerLoginDlg::onServerDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ServerLoginDlg::onServerReadyRead);
    connect(m_socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, &ServerLoginDlg::onServerError);
}

ServerLoginDlg::~ServerLoginDlg()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

void ServerLoginDlg::createUI()
{
    setFixedSize(400, 500);
    EnableMoveWindow(true);
    
    // Title
    m_labTitle = new QLabel(tr("WeCompany 登录"), this);
    m_labTitle->setAlignment(Qt::AlignCenter);
    m_labTitle->setStyleSheet("QLabel{font: bold 24px; color:#1485EE;}");
    
    // Logo
    m_labLogo = new QLabel(this);
    m_labLogo->setFixedSize(100, 100);
    m_labLogo->setStyleSheet("border-image: url(:/qss/res/WeComCopy.png);");
    
    // Username
    m_labUsername = new QLabel(tr("用户名:"), this);
    m_labUsername->setStyleSheet("QLabel{font: 14px; color:#333333;}");
    
    m_editUsername = new QLineEdit(this);
    m_editUsername->setPlaceholderText(tr("请输入用户名"));
    m_editUsername->setFixedHeight(40);
    m_editUsername->setStyleSheet(R"(
        QLineEdit {
            font: 14px;
            border: 1px solid #CCCCCC;
            border-radius: 4px;
            padding: 5px;
        }
        QLineEdit:focus {
            border: 1px solid #1485EE;
        }
    )");
    
    // Password
    m_labPassword = new QLabel(tr("密码:"), this);
    m_labPassword->setStyleSheet("QLabel{font: 14px; color:#333333;}");
    
    m_editPassword = new QLineEdit(this);
    m_editPassword->setPlaceholderText(tr("请输入密码"));
    m_editPassword->setEchoMode(QLineEdit::Password);
    m_editPassword->setFixedHeight(40);
    m_editPassword->setStyleSheet(R"(
        QLineEdit {
            font: 14px;
            border: 1px solid #CCCCCC;
            border-radius: 4px;
            padding: 5px;
        }
        QLineEdit:focus {
            border: 1px solid #1485EE;
        }
    )");
    
    // Remember me checkbox
    m_chkRememberMe = new QCheckBox(tr("记住密码"), this);
    m_chkRememberMe->setStyleSheet("QCheckBox{font: 12px; color:#666666;}");
    
    // Login button
    m_btnLogin = new QPushButton(tr("登录"), this);
    m_btnLogin->setFixedHeight(45);
    m_btnLogin->setStyleSheet(R"(
        QPushButton {
            font: bold 16px;
            color: white;
            background-color: #1485EE;
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #0D6FD4;
        }
        QPushButton:pressed {
            background-color: #0A5AB8;
        }
        QPushButton:disabled {
            background-color: #CCCCCC;
        }
    )");
    connect(m_btnLogin, &QPushButton::clicked, this, &ServerLoginDlg::onLoginClicked);
    
    // Register button
    m_btnRegister = new QPushButton(tr("注册新账号"), this);
    m_btnRegister->setFixedHeight(40);
    m_btnRegister->setStyleSheet(R"(
        QPushButton {
            font: 14px;
            color: #1485EE;
            background-color: white;
            border: 1px solid #1485EE;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #F0F8FF;
        }
        QPushButton:pressed {
            background-color: #E0F0FF;
        }
    )");
    connect(m_btnRegister, &QPushButton::clicked, this, &ServerLoginDlg::onRegisterClicked);
    
    // Status label
    m_labStatus = new QLabel(this);
    m_labStatus->setAlignment(Qt::AlignCenter);
    m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
    m_labStatus->setWordWrap(true);
    
    // Layout
    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->addStretch();
    logoLayout->addWidget(m_labLogo);
    logoLayout->addStretch();
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_labTitle);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(logoLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_labUsername);
    mainLayout->addWidget(m_editUsername);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_labPassword);
    mainLayout->addWidget(m_editPassword);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_chkRememberMe);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_btnLogin);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_btnRegister);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_labStatus);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(40, 20, 40, 20);
    
    centralWidget()->setLayout(mainLayout);
}

void ServerLoginDlg::connectToServer()
{
    m_labStatus->setText(tr("正在连接服务器..."));
    m_labStatus->setStyleSheet("QLabel{font: 12px; color:#1485EE;}");
    m_btnLogin->setEnabled(false);
    m_btnRegister->setEnabled(false);
    
    m_socket->connectToHost(m_serverHost, m_serverPort);
}

void ServerLoginDlg::onLoginClicked()
{
    m_username = m_editUsername->text().trimmed();
    QString password = m_editPassword->text();
    
    if (m_username.isEmpty()) {
        m_labStatus->setText(tr("请输入用户名"));
        m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
        return;
    }
    
    if (password.isEmpty()) {
        m_labStatus->setText(tr("请输入密码"));
        m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
        return;
    }
    
    m_isRegistering = false;
    connectToServer();
}

void ServerLoginDlg::onRegisterClicked()
{
    m_username = m_editUsername->text().trimmed();
    QString password = m_editPassword->text();
    
    if (m_username.isEmpty()) {
        m_labStatus->setText(tr("请输入用户名"));
        m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
        return;
    }
    
    if (password.isEmpty()) {
        m_labStatus->setText(tr("请输入密码"));
        m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
        return;
    }
    
    if (password.length() < 6) {
        m_labStatus->setText(tr("密码长度至少6位"));
        m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
        return;
    }
    
    m_isRegistering = true;
    connectToServer();
}

void ServerLoginDlg::onServerConnected()
{
    m_labStatus->setText(tr("已连接，正在验证..."));
    
    if (m_isRegistering) {
        sendRegisterRequest();
    } else {
        sendLoginRequest();
    }
}

void ServerLoginDlg::onServerDisconnected()
{
    m_btnLogin->setEnabled(true);
    m_btnRegister->setEnabled(true);
}

void ServerLoginDlg::sendLoginRequest()
{
    // Send login request to server
    // Protocol: [MSG_TYPE][username_len][username][password]
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(0x08);  // Login request message type
    
    QByteArray usernameData = m_username.toUtf8();
    stream << static_cast<quint16>(usernameData.size());
    stream.writeRawData(usernameData.data(), usernameData.size());
    
    QByteArray passwordData = m_editPassword->text().toUtf8();
    stream << static_cast<quint16>(passwordData.size());
    stream.writeRawData(passwordData.data(), passwordData.size());
    
    m_socket->write(message);
    m_socket->flush();
}

void ServerLoginDlg::sendRegisterRequest()
{
    // Send register request to server
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(0x09);  // Register request message type
    
    QByteArray usernameData = m_username.toUtf8();
    stream << static_cast<quint16>(usernameData.size());
    stream.writeRawData(usernameData.data(), usernameData.size());
    
    QByteArray passwordData = m_editPassword->text().toUtf8();
    stream << static_cast<quint16>(passwordData.size());
    stream.writeRawData(passwordData.data(), passwordData.size());
    
    m_socket->write(message);
    m_socket->flush();
}

void ServerLoginDlg::onServerReadyRead()
{
    QByteArray data = m_socket->readAll();
    processServerResponse(data);
}

void ServerLoginDlg::processServerResponse(const QByteArray &data)
{
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_9);
    
    quint8 msgType;
    stream >> msgType;
    
    if (msgType == 0x0A) {  // Login/Register response
        quint8 success;
        stream >> success;
        
        if (success) {
            // Read token and userId
            quint16 tokenLen;
            stream >> tokenLen;
            QByteArray tokenData(tokenLen, 0);
            stream.readRawData(tokenData.data(), tokenLen);
            m_token = QString::fromUtf8(tokenData);
            
            quint16 userIdLen;
            stream >> userIdLen;
            QByteArray userIdData(userIdLen, 0);
            stream.readRawData(userIdData.data(), userIdLen);
            m_userId = QString::fromUtf8(userIdData);
            
            m_labStatus->setText(m_isRegistering ? tr("注册成功!") : tr("登录成功!"));
            m_labStatus->setStyleSheet("QLabel{font: 12px; color:#00AA00;}");
            
            emit loginSuccessful(m_token, m_userId, m_username);
            
            QTimer::singleShot(500, this, &ServerLoginDlg::accept);
        } else {
            quint16 errorLen;
            stream >> errorLen;
            QByteArray errorData(errorLen, 0);
            stream.readRawData(errorData.data(), errorLen);
            QString error = QString::fromUtf8(errorData);
            
            m_labStatus->setText(error);
            m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
            
            m_btnLogin->setEnabled(true);
            m_btnRegister->setEnabled(true);
        }
    }
}

void ServerLoginDlg::onServerError(QAbstractSocket::SocketError error)
{
    QString errorMsg;
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            errorMsg = tr("无法连接到服务器，请检查服务器是否运行");
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMsg = tr("找不到服务器主机");
            break;
        case QAbstractSocket::NetworkError:
            errorMsg = tr("网络错误");
            break;
        default:
            errorMsg = tr("连接错误: ") + m_socket->errorString();
            break;
    }
    
    m_labStatus->setText(errorMsg);
    m_labStatus->setStyleSheet("QLabel{font: 12px; color:#FF0000;}");
    m_btnLogin->setEnabled(true);
    m_btnRegister->setEnabled(true);
}
