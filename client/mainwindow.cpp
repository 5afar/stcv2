#include "client/mainwindow.h"

#include <QAction>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTcpSocket>

#include "client/edituserdialog.h"
#include "common/protocol.h"
#include "common/validation.h"
#include "ui_mainwindow.h"

namespace {
constexpr const char* kHost = "127.0.0.1";
constexpr quint16 kPort = 5555;
}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_socket(new QTcpSocket(this)) {
    m_ui->setupUi(this);  // создание виджетов, нужно запускать после инициализации m_ui, чтобы
                          // указатели стали валидными

    // Заголовки и поведение таблицы
    m_ui->usersTable->setHorizontalHeaderLabels(
        {QStringLiteral("ID"), QStringLiteral("Username"), QStringLiteral("Email")});
    m_ui->usersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_ui->usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_ui->usersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // Ограничение длины ввода
    m_ui->usernameEdit->setMaxLength(validation::kUsernameMaxLen);
    m_ui->emailEdit->setMaxLength(validation::kEmailMaxLen);
    m_ui->usernameEdit->setPlaceholderText(QStringLiteral("safar"));
    m_ui->emailEdit->setPlaceholderText(QStringLiteral("example@exam.com"));

    // Связь сигналов сокета со слотами окна
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    // Связь кнопок со слотами
    connect(m_ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(m_ui->refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_ui->reconnectButton, &QPushButton::clicked, this, &MainWindow::onReconnectClicked);
    connect(m_ui->editButton, &QPushButton::clicked, this, &MainWindow::onEditClicked);
    connect(m_ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteClicked);

    // Контекстное меню на таблице (правый клик)
    m_ui->usersTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui->usersTable, &QTableWidget::customContextMenuRequested, this,
            &MainWindow::onTableContextMenuRequested);

    // Включение/выключение кнопок Edit/Delete по выделению строки
    connect(m_ui->usersTable, &QTableWidget::itemSelectionChanged, this,
            &MainWindow::onTableSelectionChanged);
    onTableSelectionChanged();  // стартовое состояние — выключены

    connectToServer();
}

MainWindow::~MainWindow() {
    delete m_ui;
}

// state() текущее состояние сокета
void MainWindow::connectToServer() {
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->abort();  // принудительно закрываем соединение

    setStatus(QStringLiteral("Connecting to %1:%2...").arg(QString::fromLatin1(kHost)).arg(kPort));
    m_socket->connectToHost(QString::fromLatin1(kHost), kPort);  // вернет сигнал connected
}

void MainWindow::onConnected() {
    setStatus(QStringLiteral("Connected to %1:%2").arg(QString::fromLatin1(kHost)).arg(kPort));
    refreshUsers();  // после подключения, сразу запрашиваем пользователей
}

void MainWindow::onDisconnected() {
    setStatus(QStringLiteral("Disconnected"));
}

void MainWindow::onSocketError(QAbstractSocket::SocketError) {
    setStatus(QStringLiteral("Socket error: %1").arg(m_socket->errorString()));
}
// чтеник кадров
void MainWindow::onReadyRead() {
    m_buffer.append(m_socket->readAll());  // все что пришло забираем в буфер

    QJsonObject resp;
    while (protocol::tryExtract(m_buffer,
                                resp)) {  // из полученных данных пробуем выделить целые кадры
        handleResponse(resp);
    }
}
// Добавление пользователя
void MainWindow::onAddClicked() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {  // проверка подключения
        QMessageBox::warning(this, QStringLiteral("Not connected"),
                             QStringLiteral("No connection to server"));
        return;
    }

    const QString username = m_ui->usernameEdit->text().trimmed();
    const QString email = m_ui->emailEdit->text().trimmed();

    QString validationError = validation::validateUsername(username);
    if (validationError.isEmpty())
        validationError = validation::validateEmail(email);
    if (!validationError.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid input"), validationError);
        return;
    }

    QJsonObject req;
    req[QStringLiteral("action")] = QStringLiteral("add_user");
    req[QStringLiteral("username")] = username;
    req[QStringLiteral("email")] = email;
    sendJson(req);  // заполняем и отправляем JSON
}

void MainWindow::onRefreshClicked() {
    refreshUsers();
}

void MainWindow::onReconnectClicked() {
    connectToServer();
}

void MainWindow::sendJson(const QJsonObject& obj) {
    m_socket->write(protocol::encode(obj));
}
// запрос обновления таблицы
void MainWindow::refreshUsers() {
    if (m_socket->state() != QAbstractSocket::ConnectedState)  // проверка состояния соединения
        return;
    QJsonObject req;
    req[QStringLiteral("action")] = QStringLiteral("get_users");
    sendJson(req);  // отправляем запрос
}
// Обработка ответов от сервера

/*
 * Добавление пользователя
 * {
        "action": "add_user",
        "username": "Sidorov",
        "email": "sidorov@example.com"
    }
    ответ
    {
        "status": "success",
        "message": "User added successfully"
    }

 * Запрос пользователей
 * { "action": "get_users" }
    ответ
    {
        "status": "success",
        "users": [
                    {
                    "id": 1,
                    "username": "Sidorov",
                    "email": "sidorov@example.com"
                    },
                    {
                    "id": 2,
                    "username": "Petrov",
                    "email": "petrov@example.com"
                    }
        ]
    }
 * */
void MainWindow::handleResponse(const QJsonObject& resp) {
    const QString status = resp.value(QStringLiteral("status")).toString();  // статус запроса

    if (status == QLatin1String("success")) {
        if (resp.contains(
                QStringLiteral("users"))) {  // если в запросе есть users, то это ответ на get_users
            updateUsersTable(resp.value(QStringLiteral("users")).toArray());
        } else if (resp.contains(QStringLiteral(
                       "message"))) {  // если есть message то ответ на add_user см. шаблон JSON
            setStatus(resp.value(QStringLiteral("message")).toString());
            m_ui->usernameEdit->clear();
            m_ui->emailEdit->clear();
            refreshUsers();
        }
    } else {
        QMessageBox::warning(
            this, QStringLiteral("Server error"),
            resp.value(QStringLiteral("message")).toString(QStringLiteral("Unknown error")));
    }
}
// обновление таблицы с пользователями
void MainWindow::updateUsersTable(const QJsonArray& users) {
    m_ui->usersTable->setRowCount(users.size());
    for (int i = 0; i < users.size(); ++i) {
        const QJsonObject u = users.at(i).toObject();
        m_ui->usersTable->setItem(
            i, 0, new QTableWidgetItem(QString::number(u.value(QStringLiteral("id")).toInt())));
        m_ui->usersTable->setItem(
            i, 1, new QTableWidgetItem(u.value(QStringLiteral("username")).toString()));
        m_ui->usersTable->setItem(
            i, 2, new QTableWidgetItem(u.value(QStringLiteral("email")).toString()));
    }
}

void MainWindow::setStatus(const QString& text) {
    m_ui->statusLabel->setText(text);
}

void MainWindow::onTableSelectionChanged() {
    const bool hasSelection = selectedUserId() > 0;
    m_ui->editButton->setEnabled(hasSelection);
    m_ui->deleteButton->setEnabled(hasSelection);
}

int MainWindow::selectedUserId() const {
    const auto selected = m_ui->usersTable->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return -1;

    const int row = selected.first().row();
    const QTableWidgetItem* item = m_ui->usersTable->item(row, 0);
    if (!item)
        return -1;

    bool ok = false;
    const int id = item->text().toInt(&ok);
    return ok ? id : -1;
}

void MainWindow::onTableContextMenuRequested(const QPoint& pos) {
    // Правый клик по пустому месту таблицы — ничего не делаем.
    if (!m_ui->usersTable->itemAt(pos))
        return;

    // Если строка под курсором не выделена — выделяем её.
    const QModelIndex idx = m_ui->usersTable->indexAt(pos);
    if (idx.isValid())
        m_ui->usersTable->selectRow(idx.row());

    QMenu menu(this);
    QAction* editAction = menu.addAction(QStringLiteral("Edit"));
    QAction* deleteAction = menu.addAction(QStringLiteral("Delete"));
    menu.addSeparator();
    QAction* cancelAction = menu.addAction(QStringLiteral("Cancel"));

    QAction* chosen = menu.exec(m_ui->usersTable->viewport()->mapToGlobal(pos));
    if (chosen == editAction)
        onEditClicked();
    else if (chosen == deleteAction)
        onDeleteClicked();
}

void MainWindow::onEditClicked() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, QStringLiteral("Not connected"),
                             QStringLiteral("No connection to server"));
        return;
    }

    const int id = selectedUserId();
    if (id <= 0)
        return;

    // Берём текущие значения из таблицы
    const int row = m_ui->usersTable->selectionModel()->selectedRows().first().row();
    const QString currentUsername = m_ui->usersTable->item(row, 1)->text();
    const QString currentEmail = m_ui->usersTable->item(row, 2)->text();

    EditUserDialog dlg(id, currentUsername, currentEmail, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    requestUpdateUser(dlg.id(), dlg.username(), dlg.email());
}

void MainWindow::onDeleteClicked() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, QStringLiteral("Not connected"),
                             QStringLiteral("No connection to server"));
        return;
    }

    const int id = selectedUserId();
    if (id <= 0)
        return;

    const QMessageBox::StandardButton answer =
        QMessageBox::question(this, QStringLiteral("Delete user"),
                              QStringLiteral("Delete user #%1? This cannot be undone.").arg(id),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    requestDeleteUser(id);
}

void MainWindow::requestUpdateUser(int id, const QString& username, const QString& email) {
    QJsonObject req;
    req[QStringLiteral("action")] = QStringLiteral("update_user");
    req[QStringLiteral("id")] = id;
    req[QStringLiteral("username")] = username;
    req[QStringLiteral("email")] = email;
    sendJson(req);
}

void MainWindow::requestDeleteUser(int id) {
    QJsonObject req;
    req[QStringLiteral("action")] = QStringLiteral("delete_user");
    req[QStringLiteral("id")] = id;
    sendJson(req);
}