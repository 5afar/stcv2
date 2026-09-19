#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;  // Поймал ошибку с переименованным объектом в дизайнере
}
QT_END_NAMESPACE

class QTcpSocket;

class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

   private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

    void onAddClicked();
    void onRefreshClicked();
    void onReconnectClicked();

    void onEditClicked();
    void onDeleteClicked();
    void onTableContextMenuRequested(const QPoint& pos);
    void onTableSelectionChanged();

   private:
    void connectToServer();                 // Подключение к серверу из конструктора либо от кнопки
    void sendJson(const QJsonObject& obj);  // отправка
    void refreshUsers();                    // формирует запрос getUsers
    void handleResponse(const QJsonObject& resp);    // Обработка ответов от сервера
    void updateUsersTable(const QJsonArray& users);  // обновление таблицы в ui
    void setStatus(const QString& text);             // обновляем лейбл статуса
    void requestUpdateUser(int id, const QString& username,
                           const QString& email);  // Обновление пользователя
    void requestDeleteUser(int id);                // Удаление пользователя

    // Возвращает id выделенной строки или -1, если ничего не выделено.
    int selectedUserId() const;

    Ui::MainWindow* m_ui = nullptr;
    QTcpSocket* m_socket = nullptr;
    QByteArray m_buffer;
};

#endif  // MAINWINDOW_H
