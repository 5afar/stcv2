#include "server/requesttask.h"

#include <QJsonArray>
#include <QMetaObject>

#include "common/validation.h"
#include "server/clientconnection.h"
#include "server/database.h"

// Эти функции нужны только внутри файла, через namespace прячем их от остальной программы
namespace {

QJsonObject successReply(const QString& message = {}) {
    QJsonObject o;
    o[QStringLiteral("status")] = QStringLiteral("success");
    if (!message.isEmpty())
        o[QStringLiteral("message")] = message;
    return o;
}

QJsonObject errorReply(const QString& message) {
    QJsonObject o;
    o[QStringLiteral("status")] = QStringLiteral("error");
    o[QStringLiteral("message")] = message;
    return o;
}
}  // namespace

RequestTask::RequestTask(const QJsonObject& req, const QPointer<ClientConnection>& conn)
    : m_req(req)
    , m_conn(conn) {
    setAutoDelete(true);  // После выполнения run() задачу можно удалить
}

void RequestTask::run() {
    QJsonObject reply;
    const QString action = m_req.value(QStringLiteral("action")).toString();

    if (action == QStringLiteral("add_user")) {
        const QString username = m_req.value(QStringLiteral("username")).toString().trimmed();
        const QString email = m_req.value(QStringLiteral("email")).toString().trimmed();

        QString validationError = validation::validateUsername(username);
        if (validationError.isEmpty())
            validationError = validation::validateEmail(email);
        if (!validationError.isEmpty()) {
            reply = errorReply(validationError);
        } else {
            int newId = -1;
            QString err;
            if (Database::instance().addUser(username, email, &newId, &err)) {
                reply = successReply(QStringLiteral("User added successfully"));
                reply[QStringLiteral("id")] = newId;
            } else {
                reply = errorReply(err);
            }
        }
    } else if (action == QStringLiteral("get_users")) {
        QString err;
        const auto users = Database::instance().getUsers(&err);
        if (!err.isEmpty()) {
            reply = errorReply(err);
        } else {
            reply = successReply();
            QJsonArray arr;
            for (const auto& u : users) {
                QJsonObject jo;
                jo[QStringLiteral("id")] = u.id;
                jo[QStringLiteral("username")] = u.username;
                jo[QStringLiteral("email")] = u.email;
                arr.append(jo);
            }
            reply[QStringLiteral("users")] = arr;
        }
    } else {
        reply = errorReply(QStringLiteral("Unknown action: %1").arg(action));
    }
    // invokeMethod работает через метаданные QT, создается событие типа QMetaCallEvent и кладется в
    // очередь событий потока m_conn
    if (m_conn) {  // в m_conn у нас живет соединение с клиентом
        QMetaObject::invokeMethod(m_conn, "writeResponse", Qt::QueuedConnection,
                                  Q_ARG(QJsonObject, reply));
    }
}