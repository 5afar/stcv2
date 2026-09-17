#include <QCoreApplication>
#include <QDebug>

#include "server/database.h"

static void printUsers(const QVector<User>& users) {
    qInfo() << "Users in DB:" << users.size();
    for (const User& u : users) {
        qInfo() << " id=" << u.id << " username=" << u.username << " email=" << u.email;
    }
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    QString err;
    if (!Database::instance().init(QStringLiteral("test_users.db"), &err)) {
        qCritical() << "DB init failed: " << err;
        return 1;
    }
    qInfo() << "DB initialized";

    printUsers(Database::instance().getUsers());

    int newId = -1;
    if (!Database::instance().addUser(QStringLiteral("Ivanov"), QStringLiteral("Ivanov@ivan.ru"),
                                      &newId, &err)) {
        qCritical() << "addUser failed: " << err;
        return 1;
    }
    qInfo() << "Add user with id: " << newId;

    if (!Database::instance().addUser(QStringLiteral("Petrov"), QStringLiteral("petrov@petr.ru"),
                                      &newId, &err)) {
        qCritical() << "addUser failed: " << err;
        return 1;
    }
    qInfo() << "Add user with id: " << newId;

    printUsers(Database::instance().getUsers());

    return 0;
}