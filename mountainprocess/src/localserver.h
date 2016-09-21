#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QLocalSocket>
#include <QLocalServer>

namespace LocalServer {

class Server;
class Client;

class Handler : public QObject {
public:
    Handler(Client* client, QObject *parent = 0);
    ~Handler();
protected:
    virtual bool dispatchMessage(const QByteArray &ba);
    Client* client() const { return m_client; }
private:
    Client* m_client;
};

class Client : public QObject {
    Q_OBJECT
public:
    Client(QLocalSocket *sock, Server* parent);

    ~Client() {
        socket()->deleteLater();
    }
    void writeMessage(const QByteArray &ba) {
        QByteArray message = ba;
        // prepend message size
        QByteArray sizeArray(4, 0);
        uint32_t size = ba.size();
        std::copy((char*)&size, (char*)&size+4, sizeArray.data());
        message.prepend(sizeArray);
        socket()->write(message);
        socket()->flush();
    }

    void write(const QByteArray &ba) {
        socket()->write(ba);
    }
    void close() {
        socket()->disconnectFromServer();
    }

signals:
    void disconnected();

protected:
    QLocalSocket *socket() const { return m_socket; }
    Server* server() const { return m_server; }
    virtual bool handleMessage(const QByteArray &) { return true; }
    void messageLoop() {
        uint32_t msgSize;
        while (true) {
            if (m_buffer.size() < 4) return;
            std::copy(m_buffer.constData(), m_buffer.constData()+4, (char*)&msgSize);
            if ((uint32_t)m_buffer.size() < 4+msgSize) return;
            // read message
            QByteArray message = m_buffer.mid(4, msgSize);
            m_buffer.remove(0, msgSize+4);
            handleMessage(message);
            // rinse and repeat
        }
    }

private slots:
    void handleDisconnected();
    void readData();

private:
    QByteArray m_buffer;
    QLocalSocket *m_socket;
    Server* m_server;
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server(){
        foreach(Client *c, m_clients) {
            c->disconnect(this);
            c->close();
        }
        m_clients.clear();
    }
    bool listen(const QString &path);
    void shutdown();
signals:

public slots:
protected:
    QLocalServer* socket() const { return m_socket; }
    virtual Client* createClient(QLocalSocket *sock) {
        return new Client(sock, this);
    }
    void broadcast(const QByteArray &message) {
        foreach(Client *client, m_clients) {
            client->writeMessage(message);
        }
    }

private slots:
    void handleNewConnection() {
        QLocalSocket *s = socket()->nextPendingConnection();
        Client *client = createClient(s);
        m_clients.append(client);
        connect(client, &Client::disconnected, [this, client]() { handleClientDisconnected(client); });

    }
    void handleClientDisconnected(Client *client) {
        m_clients.removeOne(client);
        client->deleteLater();
    }

private:
    QList<Client*> m_clients;
    QLocalServer* m_socket;
};

}


namespace LocalClient {

#if 0
class Request {

private:
    uint32_t m_id;
};

class Response : public QIODevice {
    Q_OBJECT
public:
signals:
    void finished();
};
#endif

class Client : public QObject {
    Q_OBJECT
public:
    Client(QObject *parent = 0) : QObject(parent) {
        m_socket = new QLocalSocket(this);
        connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));
        connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(readData()));
        connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
                this, SIGNAL(error(QLocalSocket::LocalSocketError)));
    }

    void connectToServer(const QString &path) {
        m_socket->connectToServer(path);
    }
    QLocalSocket::LocalSocketState state() const { return m_socket->state(); }
    bool isConnected() const {
        return (m_socket->state() != QLocalSocket::UnconnectedState);
    }
    void close() {
        m_socket->disconnectFromServer();
    }
    void writeMessage(const QByteArray &ba) {
        QByteArray message = ba;
        // prepend message size
        QByteArray sizeArray(4, 0);
        uint32_t size = ba.size();
        std::copy((char*)&size, (char*)&size+4, sizeArray.data());
        message.prepend(sizeArray);
        socket()->write(message);
        socket()->flush();
    }
    bool waitForConnected(int ms = 30000) {
        return socket()->waitForConnected(ms);
    }
    bool waitForReadyRead(int ms = 30000) {
        return socket()->waitForReadyRead(ms);
    }

signals:
    void connected();
    void disconnected();
    void error(QLocalSocket::LocalSocketError);
protected:
    QLocalSocket* socket() const { return m_socket; }
    virtual void handleMessage(const QByteArray &ba) {}
    void messageLoop() {
        uint32_t msgSize;
        while (true) {
            if (m_buffer.size() < 4) return;
            std::copy(m_buffer.constData(), m_buffer.constData()+4, (char*)&msgSize);
            if ((uint32_t)m_buffer.size() < 4+msgSize) return;
            // read message
            QByteArray message = m_buffer.mid(4, msgSize);
            m_buffer.remove(0, msgSize+4);
            handleMessage(message);
            // rinse and repeat
        }
    }
private slots:
    void readData() {
        m_buffer.append(socket()->readAll());
        messageLoop();
    }

private:
    QLocalSocket *m_socket;
    QByteArray m_buffer;
};
}

#endif // LOCALSERVER_H
