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
bool validateCredentials(const QString& username, const QString& email, QString* err) {
    QString e = validation::validateUsername(username);
    if (e.isEmpty())
        e = validation::validateEmail(email);
    if (!e.isEmpty()) {
        if (err)
            *err = e;
        return false;
    }
    return true;
}
}  // namespace

RequestTask::RequestTask(const QJsonObject& req, const QPointer<ClientConnection>& conn)
    : m_req(req)
    , m_conn(conn) {
    setAutoDelete(true);  // После выполнения run() задачу можно удалить
}
void RequestTask::run() {
    const QString action = m_req.value(QStringLiteral("action")).toString();

    QJsonObject reply;
    if (action == QLatin1String("add_user"))
        reply = handleAddUser();
    else if (action == QLatin1String("update_user"))
        reply = handleUpdateUser();
    else if (action == QLatin1String("delete_user"))
        reply = handleDeleteUser();
    else if (action == QLatin1String("get_users"))
        reply = handleGetUsers();
    else
        reply = errorReply(QStringLiteral("Unknown action: %1").arg(action));

    // invokeMethod работает через метаданные QT, создается событие типа QMetaCallEvent и кладется в
    // очередь событий потока m_conn
    if (m_conn) {  // в m_conn у нас живет соединение с клиентом
        QMetaObject::invokeMethod(m_conn, "writeResponse", Qt::QueuedConnection,
                                  Q_ARG(QJsonObject, reply));
    }
}
QJsonObject RequestTask::handleAddUser() {
    const QString username = m_req.value(QStringLiteral("username")).toString().trimmed();
    const QString email = m_req.value(QStringLiteral("email")).toString().trimmed();

    QString validationError;
    if (!validateCredentials(username, email, &validationError))
        return errorReply(validationError);

    const QString uniqueError = Database::instance().checkUnique(username, email);
    if (!uniqueError.isEmpty())
        return errorReply(uniqueError);

    int newId = -1;
    QString err;
    if (Database::instance().addUser(username, email, &newId, &err)) {
        QJsonObject reply = successReply(QStringLiteral("User added successfully"));
        reply[QStringLiteral("id")] = newId;
        return reply;
    }

    if (err.contains(QStringLiteral("UNIQUE"), Qt::CaseInsensitive))
        return errorReply(QStringLiteral("Username or email already exists"));
    return errorReply(err);
}

QJsonObject RequestTask::handleUpdateUser() {
    const int id = m_req.value(QStringLiteral("id")).toInt(-1);
    if (id <= 0)
        return errorReply(QStringLiteral("Missing or invalid 'id'"));

    const QString username = m_req.value(QStringLiteral("username")).toString().trimmed();
    const QString email = m_req.value(QStringLiteral("email")).toString().trimmed();

    QString validationError;
    if (!validateCredentials(username, email, &validationError))
        return errorReply(validationError);

    QString err;
    if (!Database::instance().updateUser(id, username, email, &err))
        return errorReply(err);

    return successReply(QStringLiteral("User updated successfully"));
}

QJsonObject RequestTask::handleDeleteUser() {
    const int id = m_req.value(QStringLiteral("id")).toInt(-1);
    if (id <= 0)
        return errorReply(QStringLiteral("Missing or invalid 'id'"));

    QString err;
    if (!Database::instance().deleteUser(id, &err))
        return errorReply(err);

    return successReply(QStringLiteral("User deleted successfully"));
}

QJsonObject RequestTask::handleGetUsers() {
    QString err;
    const auto users = Database::instance().getUsers(&err);
    if (!err.isEmpty())
        return errorReply(err);

    QJsonArray arr;
    for (const auto& u : users) {
        QJsonObject jo;
        jo[QStringLiteral("id")] = u.id;
        jo[QStringLiteral("username")] = u.username;
        jo[QStringLiteral("email")] = u.email;
        arr.append(jo);
    }
    QJsonObject reply = successReply();
    reply[QStringLiteral("users")] = arr;
    return reply;
}