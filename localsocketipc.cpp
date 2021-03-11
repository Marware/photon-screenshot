#include "localsocketipc.h"

#include <QtNetwork>
#include <QDebug>


#ifdef Q_OS_WIN
#include <windows.h>
#endif

LocalSocketIpcClient::LocalSocketIpcClient(QString remoteServername, QObject *parent) :
        QObject(parent) {

    m_socket = new QLocalSocket(this);
    m_serverName = remoteServername;

    connect(m_socket, SIGNAL(connected()), this, SLOT(socket_connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socket_readReady()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(socket_error(QLocalSocket::LocalSocketError)));
}

LocalSocketIpcClient::~LocalSocketIpcClient() {
    m_socket->abort();
    delete m_socket;
    m_socket = NULL;
}

void LocalSocketIpcClient::send_MessageToServer(QString message) {
    m_socket->abort();
    m_message = message;
    m_socket->connectToServer(m_serverName);
}

void LocalSocketIpcClient::socket_connected(){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << m_message;
    out.device()->seek(0);
    m_socket->write(block);
    m_socket->flush();
}

void LocalSocketIpcClient::socket_disconnected() {
    qDebug() << "socket_disconnected";
}


void LocalSocketIpcClient::socket_readReady() {
    qDebug() << "socket_readReady";
}

void LocalSocketIpcClient::socket_error(QLocalSocket::LocalSocketError) {
    qDebug() << "socket_error";
}

//////////////////////////////////////////////////////////////////////////////////////////////

LocalSocketIpcServer::LocalSocketIpcServer(QString servername, QObject *parent)
    :QObject(parent) {
    m_server = new QLocalServer(this);
    if (!m_server->listen(servername)) {
        qDebug() << "Not able to start the Server";
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(socket_new_connection()));

}

LocalSocketIpcServer::~LocalSocketIpcServer() {

}


void LocalSocketIpcServer::socket_new_connection() {
int tries=0;
    QLocalSocket *clientConnection = m_server->nextPendingConnection();
    while (clientConnection->bytesAvailable() < (int)sizeof(quint32))
    {
        clientConnection->waitForReadyRead(1000);
        tries++;
        if(tries==100)
        {
            //break;
            //socket_new_connection();
            return;
        }
        //Sleep(1);
    }
    tries = 0;
        qWarning() << "SOCK" << "3";
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    qWarning() << "SOCK" << "4";
    QDataStream in(clientConnection);
    if (clientConnection->bytesAvailable() < (int)sizeof(quint16)) {
        qWarning() << "SOCK" << "5";
        return;
    }
    QString message;
    in >> message;
qWarning() << "SOCK" << "6";// << message;
    emit messageReceived(message);
}
