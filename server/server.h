#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

/// наследуемся чтобы соединения отправлять в новые потоки(QT поумолчанию создает в текущем)
class Server : public QTcpServer {
    Q_OBJECT
   public:
    explicit Server(QObject* parent = nullptr);  /// explicit запрещает неявное преобразование

   protected:
    void incomingConnection(qintptr socketDescriptor)
        override;  /// override указывает, что мы переопределяем метод, а не создаем новый
};

#endif  // SERVER_H
