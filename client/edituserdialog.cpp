#include "client/edituserdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "common/validation.h"

EditUserDialog::EditUserDialog(int id, const QString& username, const QString& email,
                               QWidget* parent)
    : QDialog(parent)
    , m_id(id) {
    setWindowTitle(QStringLiteral("Edit user #%1").arg(id));
    setModal(true);

    m_usernameEdit = new QLineEdit(username, this);
    m_usernameEdit->setMaxLength(validation::kUsernameMaxLen);

    m_emailEdit = new QLineEdit(email, this);
    m_emailEdit->setMaxLength(validation::kEmailMaxLen);

    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Username:"), m_usernameEdit);
    form->addRow(QStringLiteral("Email:"), m_emailEdit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    connect(buttons, &QDialogButtonBox::accepted, this, &EditUserDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(buttons);

    resize(420, 140);
}

QString EditUserDialog::username() const {
    return m_usernameEdit->text().trimmed();
}

QString EditUserDialog::email() const {
    return m_emailEdit->text().trimmed();
}

void EditUserDialog::onAccepted() {
    const QString u = username();
    const QString e = email();

    QString validationError = validation::validateUsername(u);
    if (validationError.isEmpty())
        validationError = validation::validateEmail(e);

    if (!validationError.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid input"), validationError);
        return;  // диалог остаётся открытым
    }

    accept();  // закрывает диалог с результатом QDialog::Accepted
}