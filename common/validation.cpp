#include "common/validation.h"

#include <QRegularExpression>

namespace validation {

namespace {
bool hasControlChars(const QString& s) {
    for (const QChar ch : s) {
        if (ch.category() == QChar::Other_Control)
            return true;
    }
    return false;
}

// Регулярка для email
// Требует: <что-то>@<что-то>.<что-то-из-2+символов>
// Не допускает пробелов, запятых, точек подряд, точек в начале/конце.
const QRegularExpression& emailRegex() {
    static const QRegularExpression re(
        QStringLiteral(R"(^[A-Za-z0-9](?:[A-Za-z0-9._%+\-]*[A-Za-z0-9])?)"
                       R"(@)"
                       R"([A-Za-z0-9](?:[A-Za-z0-9\-]*[A-Za-z0-9])?)"
                       R"((?:\.[A-Za-z0-9](?:[A-Za-z0-9\-]*[A-Za-z0-9])?)+$)"));
    return re;
}
}  // namespace

QString validateUsername(const QString& username) {
    if (username.isEmpty())
        return QStringLiteral("Username must not be empty");

    if (username.length() < kUsernameMinLen)
        return QStringLiteral("Username is too short");

    if (username.length() > kUsernameMaxLen)
        return QStringLiteral("Username is too long (max %1 characters)").arg(kUsernameMaxLen);

    if (hasControlChars(username))
        return QStringLiteral("Username contains invalid characters");

    if (username.front().isSpace() || username.back().isSpace())
        return QStringLiteral("Username must not start or end with whitespace");

    return {};
}

QString validateEmail(const QString& email) {
    if (email.isEmpty())
        return QStringLiteral("Email must not be empty");

    if (email.length() < kEmailMinLen)
        return QStringLiteral("Email is too short");

    if (email.length() > kEmailMaxLen)
        return QStringLiteral("Email is too long (max %1 characters)").arg(kEmailMaxLen);

    if (hasControlChars(email))
        return QStringLiteral("Email contains invalid characters");

    if (email.contains(QLatin1Char(' ')) || email.contains(QLatin1Char('\t')))
        return QStringLiteral("Email must not contain whitespace");

    // Ровно одна '@'
    const int atCount = email.count(QLatin1Char('@'));
    if (atCount != 1)
        return QStringLiteral("Email must contain exactly one '@'");

    const int atIndex = email.indexOf(QLatin1Char('@'));
    if (atIndex == 0)
        return QStringLiteral("Email local part must not be empty");
    if (atIndex == email.length() - 1)
        return QStringLiteral("Email domain must not be empty");

    // Полная проверка формата через регулярку
    if (!emailRegex().match(email).hasMatch())
        return QStringLiteral("Email format is invalid (expected name@domain.tld)");

    return {};
}
}  // namespace validation