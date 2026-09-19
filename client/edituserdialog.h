#ifndef EDITUSERDIALOG_H
#define EDITUSERDIALOG_H
#include <QDialog>

class QLineEdit;

// диалог для редактирования пользователя
class EditUserDialog : public QDialog {
    Q_OBJECT
   public:
    EditUserDialog(int id, const QString& username, const QString& email,
                   QWidget* parent = nullptr);

    int id() const {
        return m_id;
    }
    QString username() const;
    QString email() const;

   private slots:
    // Слот вызывается при нажатии OK.
    // Проверяет поля теми же правилами, что и сервер.
    // Если валидация не прошла — показывает QMessageBox и НЕ закрывает диалог.
    void onAccepted();

   private:
    int m_id;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_emailEdit = nullptr;
};
#endif  // EDITUSERDIALOG_H
