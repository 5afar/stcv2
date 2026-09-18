#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QAbstractSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QObject>

class QTcpSocket;  /// forward declaration

// Обработчик одного соединения
//
// Живёт в отдельном QThread (создаётся в Server::incomingConnection
// и переезжает через moveToThread)
//
// Читает из сокета кадры формата [4 байта длины][JSON] через
// protocol::tryExtract. На каждый готовый кадр запускает RequestTask
// в глобальном QThreadPool. Ответы из RequestTask приходят обратно
// сюда через queued-вызов writeResponse.
class ClientConnection : public QObject {
    Q_OBJECT
   public:
    explicit ClientConnection(qintptr socketDescriptor, QObject* parent = nullptr);
    ~ClientConnection() override;

   signals:
    // Испускается, когда клиент отключился или произошла
    // ошибка. Ловится в Server, чтобы завершить поток
    void finished();

   public slots:
    // Вызывается из QThread::started — в потоке, куда был перемещён объект
    // Создаёт QTcpSocket, привязывает его к дескриптору и подключает сигналы
    void start();
    void writeResponse(const QJsonObject& resp);

   private slots:
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);

   private:
    qintptr m_descriptor;
    QTcpSocket* m_socket = nullptr;
    QByteArray m_buffer;
};

#endif  // CLIENTCONNECTION_H
