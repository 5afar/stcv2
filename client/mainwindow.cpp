#include "client/mainwindow.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTcpSocket>

#include "common/protocol.h"
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

    // Связь сигналов сокета со слотами окна
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    // Связь кнопок со слотами
    connect(m_ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(m_ui->refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_ui->reconnectButton, &QPushButton::clicked, this, &MainWindow::onReconnectClicked);

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
    if (username.isEmpty() || email.isEmpty()) {  // если поля пустые
        QMessageBox::information(this, QStringLiteral("Input"),
                                 QStringLiteral("Please fill in both fields"));
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