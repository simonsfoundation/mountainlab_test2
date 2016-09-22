#include "localserver.h"

namespace LocalServer {
Server::Server(QObject* parent)
    : QObject(parent)
{
    m_socket = new QLocalServer(this);
    connect(m_socket, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}

bool Server::listen(const QString& path)
{
    QLocalServer::removeServer(path);
    return socket()->listen(path);
}

void Server::shutdown()
{
    socket()->close();
}

Client::Client(QLocalSocket* sock, LocalServer::Server* parent)
    : QObject(parent)
    , m_socket(sock)
    , m_server(parent)
{
    connect(socket(), SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
    connect(socket(), SIGNAL(readyRead()), this, SLOT(readData()));
}

void Client::handleDisconnected()
{
    //server()->handleClientDisconnected(this);
    emit disconnected();
}

void Client::readData()
{
    m_buffer.append(socket()->readAll());
    messageLoop();
}
}
