#ifndef VALIDATION_H
#define VALIDATION_H
#include <QString>

// Общие правила валидации пользовательских полей
//
// функции возвращают пустую строку при успехе
// и текст ошибки при неудаче
namespace validation {

constexpr int kUsernameMinLen = 1;
constexpr int kUsernameMaxLen = 64;

constexpr int kEmailMinLen = 5;
constexpr int kEmailMaxLen = 254;

QString validateUsername(const QString& username);

QString validateEmail(const QString& email);

}  // namespace validation
#endif  // VALIDATION_H
