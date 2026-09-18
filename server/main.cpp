#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

#include "server/database.h"
#include "server/server.h"

static void printUsers(const QVector<User>& users) {
    qInfo() << "Users in DB:" << users.size();
    for (const User& u : users) {
        qInfo() << " id=" << u.id << " username=" << u.username << " email=" << u.email;
    }
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("users-server"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Multithreaded TCP server for user management"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOpt({QStringLiteral("p"), QStringLiteral("port")},
                               QStringLiteral("Port to listen on"), QStringLiteral("port"),
                               QStringLiteral("5555"));
    QCommandLineOption dbOpt({QStringLiteral("d"), QStringLiteral("db")},
                             QStringLiteral("Path to SQLite database file"), QStringLiteral("path"),
                             QStringLiteral("users.db"));

    parser.addOption(portOpt);
    parser.addOption(dbOpt);
    parser.process(app);

    const quint16 port = static_cast<quint16>(parser.value(portOpt).toUShort());
    const QString dbPath = parser.value(dbOpt);

    QString err;
    if (!Database::instance().init(dbPath, &err)) {
        qCritical("Database init failed: %s", qPrintable(err));
        return 1;
    }

    Server server;
    if (!server.listen(QHostAddress::Any, port)) {
        qCritical("Listen failed: %s", qPrintable(server.errorString()));
        return 1;
    }

    qInfo("Server listening on port %u", unsigned(port));
    qInfo("Database: %s", qPrintable(QFileInfo(dbPath).absoluteFilePath()));

    return app.exec();
}