#ifndef LOCALSOCKETIPC_H
#define LOCALSOCKETIPC_H

#include <QObject>
#include <QtNetwork>


class LocalSocketIpcClient : public QObject
{
    Q_OBJECT
public:
    LocalSocketIpcClient(QString remoteServername, QObject *parent = 0);
    ~LocalSocketIpcClient();

public:
signals:

public slots:
    void send_MessageToServer(QString message);

    void socket_connected();
    void socket_disconnected();

    void socket_readReady();
    void socket_error(QLocalSocket::LocalSocketError);

private:
    QLocalSocket*   m_socket;
    quint16         m_blockSize;
    QString         m_message;
    QString         m_serverName;
};

class LocalSocketIpcServer: public QObject
{
    Q_OBJECT
public:
    LocalSocketIpcServer(QString servername, QObject *parent);
    ~LocalSocketIpcServer();


signals:
    void messageReceived(QString);

public slots:
    void socket_new_connection();

private:
    QLocalServer*       m_server;
};

#endif // LOCALSOCKETIPC_H
