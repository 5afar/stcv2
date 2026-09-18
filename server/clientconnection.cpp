#include "server/clientconnection.h"

#include <QPointer>
#include <QTcpSocket>
#include <QThreadPool>

#include "common/protocol.h"
#include "server/requesttask.h"

ClientConnection::ClientConnection(qintptr socketDescriptor, QObject* parent)
    : QObject(parent)
    , m_descriptor(socketDescriptor) {}

ClientConnection::~ClientConnection() = default;

void ClientConnection::start() {
    m_socket = new QTcpSocket(this);
    if (!m_socket->setSocketDescriptor(m_descriptor)) {
        qWarning("ClientConnection: setSocketDescriptor failed");
        emit finished();
        return;
    }

    connect(m_socket, &QTcpSocket::readyRead, this, &ClientConnection::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientConnection::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ClientConnection::onError);
}

void ClientConnection::onReadyRead() {
    m_buffer.append(m_socket->readAll());

    QJsonObject req;
    while (protocol::tryExtract(m_buffer, req)) {
        QThreadPool::globalInstance()->start(
            new RequestTask(req, QPointer<ClientConnection>(this)));
    }
}

void ClientConnection::writeResponse(const QJsonObject& resp) {
    if (!m_socket)
        return;
    if (m_socket->state() != QAbstractSocket::ConnectedState)
        return;
    m_socket->write(protocol::encode(resp));
}

void ClientConnection::onDisconnected() {
    emit finished();
}

void ClientConnection::onError(QAbstractSocket::SocketError) {
    qWarning("ClientConnection: socket error: %s",
             qPrintable(m_socket ? m_socket->errorString() : QString()));
    emit finished();
}