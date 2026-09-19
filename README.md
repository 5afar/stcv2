# User Client-Server

Клиент-серверное приложение на C++/Qt: многопоточный TCP-сервер с SQLite
и GUI-клиент на Qt Widgets

> Учебный проект для демонстрации навыков: CMake, Qt Network, Qt SQL,
> Qt Widgets, многопоточность, клиент-серверный протокол

---

## Возможности

### Сервер
- Многопоточный TCP-сервер: соединение на `QThread`, запрос на `QThreadPool`
- SQLite-хранилище (`users(id, username, email)`)
- `UNIQUE INDEX` на username и email
- Валидация входных данных: длина, формат email, control chars, пробелы
- Защита от SQL-инъекций через `QSqlQuery::prepare` + `addBindValue`
- Ограничение максимального размера сообщения (1 МиБ)

### Клиент
- Qt Widgets GUI на базе `.ui`-файла из Qt Designer
- Асинхронный `QTcpSocket` в главном потоке
- Форма добавления пользователя: username, email, валидация до отправки
- Таблица пользователей с контекстным меню и кнопками
- **Редактирование** через модальный диалог
- **Удаление** с подтверждением
- Автоматическая загрузка списка при подключении и обновление после изменений
- Кнопки активируются/деактивируются в зависимости от выделения и состояния соединения

### Протокол
 `[4 байта длины big-endian][UTF-8 JSON]`.
- Четыре действия: `add_user`, `get_users`, `update_user`, `delete_user`.
- Общая библиотека `protocol` — единая логика кодирования/декодирования
  для клиента и сервера.
---

## Требования

- CMake ≥ 3.16
- Компилятор с поддержкой C++17
- Qt 6 (проверено на Qt 6.11.2, MSYS2/MinGW-w64)
  - Модули: Core, Network, Sql, Widgets
  - Драйвер `QSQLITE` (идёт в поставке `qt6-base`)

### Установка окружения (MSYS2)

```bash
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-qt6-base \
          mingw-w64-x86_64-qt6-tools
```

### Ubuntu

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-tools-dev
```

---

## Сборка

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Появятся два исполняемых файла:

- `build/server/users_server`
- `build/client/users_client`

---

## Запуск

### Сервер

```bash
./build/server/users_server --port 5555 --db users.db
```

| Опция | По умолчанию | Описание |
|---|---|---|
| `-p`, `--port` | `5555` | Порт для прослушивания |
| `-d`, `--db`   | `users.db` | Путь к файлу SQLite |

Также доступны `--help` и `--version`.

Остановить — `Ctrl+C` или закрыть окно.

### Клиент

```bash
./build/client/users_client
```

Клиент автоматически подключается к `127.0.0.1:5555`.

Адрес и порт заданы константами `kHost` / `kPort` в `client/mainwindow.cpp`.


---

## Протокол

**Транспорт:** TCP

**Формат кадра:**

```
┌──────────────┬──────────────────────────────┐
│ 4 байта      │ N байт                       │
│ big-endian   │ UTF-8 JSON                   │
│ = N          │                              │
└──────────────┴──────────────────────────────┘
```

**Действия:**

### `add_user` — добавить пользователя

Запрос:
```json
{ "action": "add_user", "username": "Sidorov", "email": "sidorov@example.com" }
```

Успех:
```json
{ "status": "success", "message": "User added successfully", "id": 1 }
```

Ошибка (валидация, дубликат, БД):
```json
{ "status": "error", "message": "Email format is invalid (expected name@domain.tld)" }
```

### `get_users` — список всех пользователей

Запрос:
```json
{ "action": "get_users" }
```

Ответ:
```json
{
  "status": "success",
  "users": [
    { "id": 1, "username": "Sidorov", "email": "sidorov@example.com" },
    { "id": 2, "username": "Petrov",  "email": "petrov@example.com"  }
  ]
}
```

### `update_user` — изменить пользователя

Запрос:
```json
{ "action": "update_user", "id": 1, "username": "Sidorov", "email": "sidorov.new@example.com" }
```

Успех:
```json
{ "status": "success", "message": "User updated successfully" }
```

### `delete_user` — удалить пользователя

Запрос:
```json
{ "action": "delete_user", "id": 1 }
```

Успех:
```json
{ "status": "success", "message": "User deleted successfully" }
```


___

## Структура проекта

```
stcv2/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── .clang-format
├── common/
│   ├── protocol.h / .cpp       # общий формат кадров
│   └── validation.h / .cpp     # общие правила валидации
├── server/
│   ├── CMakeLists.txt
│   ├── main.cpp                # CLI-аргументы, запуск сервера
│   ├── server.h / .cpp         # QTcpServer наследник
│   ├── clientconnection.h / .cpp   # per-client обработчик
│   ├── requesttask.h / .cpp        # per-request задача (dispatcher)
│   └── database.h / .cpp           # SQLite-обёртка
└── client/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── mainwindow.h / .cpp / .ui   # главное окно
    └── edituserdialog.h / .cpp     # диалог редактирования
```

---
