#include "server/server.h"

#include <QThread>

#include "server/clientconnection.h"

Server::Server(QObject* parent)
    : QTcpServer(parent) {}

void Server::incomingConnection(qintptr socketDescriptor) {
    auto* thread = new QThread(this);
    auto* conn = new ClientConnection(socketDescriptor);

    conn->moveToThread(thread);

    connect(thread, &QThread::started, conn, &ClientConnection::start);
    connect(conn, &ClientConnection::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, conn, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
    qInfo("Accepted client, worker started");
}
