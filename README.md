# User Client-Server

Клиент-серверное приложение на C++/Qt: многопоточный TCP-сервер с SQLite
и GUI-клиент на Qt Widgets.

> Учебный проект для демонстрации навыков: CMake, Qt Network, Qt SQL,
> Qt Widgets, многопоточность, клиент-серверный протокол.

---

## Возможности

- Многопоточный TCP-сервер: каждое соединение обслуживается в отдельном
  `QThread`, каждый запрос — в отдельном воркере `QThreadPool`.
- Хранение пользователей в SQLite (`users(id, username, email)`).
- Простой протокол поверх TCP:
  `[4 байта длины big-endian][UTF-8 JSON]`.

---

## Требования

- CMake ≥ 3.16
- Компилятор с поддержкой C++17
- Qt 6 (проверено на Qt 6.11.2, MSYS2/MinGW-w64)
  - Модули: Core, Network, Sql, Widgets
  - Драйвер `QSQLITE` (идёт в поставке `qt6-base`)

Сборка через MSYS2:

```bash
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-qt6-base \
          mingw-w64-x86_64-qt6-tools
```

Сборка на Ubuntu:

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
- `build/client/users_client` (в разработке)

---

## Запуск сервера

```bash
./build/server/users_server --port 5555 --db users.db
```

Аргументы:

| Опция | По умолчанию | Описание |
|---|---|---|
| `-p`, `--port` | `5555` | Порт для прослушивания |
| `-d`, `--db` | `users.db` | Путь к файлу SQLite |

Также доступны:

```bash
./build/server/users_server --help
./build/server/users_server --version
```

Остановить сервер — `Ctrl+C` или закрыть окно.

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

Успешный ответ:

```json
{ "status": "success", "message": "User added successfully", "id": 1 }
```

Ошибка:

```json
{ "status": "error", "message": "username and email must be non-empty" }
```

### `get_users` — получить всех пользователей

Запрос:

```json
{ "action": "get_users" }
```

Успешный ответ:

```json
{
  "status": "success",
  "users": [
    { "id": 1, "username": "Sidorov", "email": "sidorov@example.com" },
    { "id": 2, "username": "Petrov",  "email": "petrov@example.com"  }
  ]
}
```

___

## Структура проекта

```
user-client-server/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── common/
│   ├── protocol.h      # общий формат кадров
│   └── protocol.cpp
├── server/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── server.h/cpp        # QTcpServer наследник
│   ├── clientconnection.h/cpp  # per-client обработчик
│   ├── requesttask.h/cpp       # per-request задача для пула
│   └── database.h/cpp          # SQLite обёртка
└── client/
    ├── CMakeLists.txt
    └── main.cpp                # GUI (в разработке)
```

---
