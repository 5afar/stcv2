#ifndef DATABASE_H
#define DATABASE_H

#include <QHash>
#include <QMutex>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

/// Структура по таблице пользователь
struct User {
    int id{};
    QString username;
    QString email;
};

/// Для подключения PostgreSQL можно добавить структуру DbConfig с параметрами подключения
class Database {
   public:
    static Database& instance();
    bool init(const QString& path, QString* err = nullptr);

    bool addUser(const QString& username, const QString& email, int* outId = nullptr,
                 QString* err = nullptr);

    QVector<User> getUsers(QString* err = nullptr);

   private:
    Database() = default;
    Q_DISABLE_COPY(Database)

    QSqlDatabase connectionForCurrentThread();

    QString m_path;
    QMutex m_mutex;
    QHash<Qt::HANDLE, QString> m_connections;
};

#endif  // DATABASE_H
