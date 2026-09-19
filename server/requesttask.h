#ifndef REQUESTTASK_H
#define REQUESTTASK_H

#include <QJsonObject>
#include <QPointer>
#include <QRunnable>

class ClientConnection;

// Одна задача для обработки одного запроса.
// Живёт в QThreadPool — то есть выполняется в одном из воркеров пула,
// а не в потоке ClientConnection.
//
// После формирования ответа отправляет его обратно в ClientConnection
// через QMetaObject::invokeMethod(Qt::QueuedConnection), чтобы код
// выполнился в потоке клиента (где живёт сокет).
class RequestTask : public QRunnable {  /// QRunnable класс для зача в QThreadPool
   public:
    RequestTask(const QJsonObject& req,
                const QPointer<ClientConnection>&
                    conn);  /// QPointer<ClientConnection> если клиент отключится до выполнения
                            /// задачи, указатель обнулится

    void run() override;

   private:
    QJsonObject handleAddUser();
    QJsonObject handleUpdateUser();
    QJsonObject handleDeleteUser();
    QJsonObject handleGetUsers();

    QJsonObject m_req;  // создаем копию изначального запроса, тк оригинал после выхода из области
                        // перестанет существовать
    QPointer<ClientConnection> m_conn;
};

#endif  // REQUESTTASK_H
