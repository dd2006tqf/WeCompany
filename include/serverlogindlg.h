#ifndef SERVERLOGINDLG_H
#define SERVERLOGINDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTcpSocket>
#include "basedlg.h"

class ServerLoginDlg : public CBaseDlg
{
    Q_OBJECT

public:
    explicit ServerLoginDlg(QWidget *parent = nullptr);
    ~ServerLoginDlg();

    QString getToken() const { return m_token; }
    QString getUserId() const { return m_userId; }
    QString getUsername() const { return m_username; }

signals:
    void loginSuccessful(const QString &token, const QString &userId, const QString &username);
    void loginFailed(const QString &error);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onServerConnected();
    void onServerDisconnected();
    void onServerReadyRead();
    void onServerError(QAbstractSocket::SocketError error);

private:
    void createUI();
    void connectToServer();
    void sendLoginRequest();
    void sendRegisterRequest();
    void processServerResponse(const QByteArray &data);

    // UI Components
    QLabel *m_labTitle;
    QLabel *m_labLogo;
    QLabel *m_labUsername;
    QLabel *m_labPassword;
    QLineEdit *m_editUsername;
    QLineEdit *m_editPassword;
    QPushButton *m_btnLogin;
    QPushButton *m_btnRegister;
    QLabel *m_labStatus;
    QCheckBox *m_chkRememberMe;

    // Connection
    QTcpSocket *m_socket;
    QString m_serverHost;
    quint16 m_serverPort;

    // Login state
    QString m_token;
    QString m_userId;
    QString m_username;
    bool m_isRegistering;
};

#endif // SERVERLOGINDLG_H
