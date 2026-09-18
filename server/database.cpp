#include "database.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QVariant>

/// Инициализация синглтона
Database& Database::instance() {
    static Database
        db;  /// статическая переменная, создается при первом вызове и живет до конца программы
    return db;
}

/// получение соединения с БД для текущего потока
QSqlDatabase Database::connectionForCurrentThread() {
    const Qt::HANDLE tid = QThread::currentThreadId();

    QMutexLocker lock(
        &m_mutex);  /// QMutexLocker в конструкторе захватывает мьютекс, в деструкторе отпускает.
                    /// Освобождает автоматически при выходе из области видимости

    /// Если поток уже обращался к базе, возвращаем его соединение
    auto it = m_connections.find(tid);
    if (it != m_connections.end())
        return QSqlDatabase::database(it.value(), true);

    /// Если обращение впервые, создаем новое соединение
    const QString name = QStringLiteral("srv_conn_%1").arg(reinterpret_cast<quintptr>(tid));
    QSqlDatabase db = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"), name);  /// при переходе на другую базу нужно менять имя драйвера
    db.setDatabaseName(m_path);
    if (!db.open()) {
        qCritical("Cannot open SQLite database '%s': %s", qPrintable(m_path),
                  qPrintable(db.lastError().text()));
    }
    m_connections.insert(tid, name);
    return db;
}

bool Database::init(const QString& path, QString* err) {
    {
        QMutexLocker lock(&m_mutex);
        m_path = path;
    }

    QSqlDatabase db = connectionForCurrentThread();
    if (!db.isOpen()) {
        if (err)
            *err = db.lastError().text();
        return false;
    }
    QSqlQuery q(db);
    const QString ddl = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT    NOT NULL,"
        "  email    TEXT    NOT NULL"
        ")");
    if (!q.exec(ddl)) {
        if (err)
            *err = q.lastError().text();
        return false;
    }
    return true;
}

bool Database::addUser(const QString& username, const QString& email, int* outId, QString* err) {
    QSqlDatabase db = connectionForCurrentThread();
    QSqlQuery q(db);

    q.prepare(QStringLiteral("INSERT INTO users (username, email) VALUES (?, ?)"));
    q.addBindValue(username);
    q.addBindValue(email);
    if (!q.exec()) {
        if (err)
            *err = q.lastError().text();
        return false;
    }
    if (outId)
        *outId = q.lastInsertId().toInt();
    return true;
}

QVector<User> Database::getUsers(QString* err) {
    QVector<User> result;
    QSqlDatabase db = connectionForCurrentThread();
    QSqlQuery q(db);

    if (!q.exec(QStringLiteral("SELECT id,username, email FROM users ORDER BY id"))) {
        if (err)
            *err = q.lastError().text();
        return result;
    }
    while (q.next()) {
        User u;
        u.id = q.value(0).toInt();
        u.username = q.value(1).toString();
        u.email = q.value(2).toString();
        result.push_back(u);
    }
    return result;
}