#include "common/protocol.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QtEndian>

namespace protocol {
QByteArray encode(const QJsonObject& obj) {
    /// QJsonDocument::Compact убирает лишние переносы и пробелы
    const QByteArray jsonBytes = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = static_cast<quint32>(jsonBytes.size());

    QByteArray frame;
    frame.resize(4);
    qToBigEndian<quint32>(len,
                          reinterpret_cast<uchar*>(frame.data()));  /// qt функция для BigEndian
    frame.append(jsonBytes);
    return frame;
}

bool tryExtract(QByteArray& buffer, QJsonObject& out) {
    if (buffer.size() < 4)
        return false;

    const quint32 len = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(buffer.constData()));

    if (len == 0 || len > kMaxMessageSize) {
        buffer.clear();  // протокол нарушен — не даём парсеру зациклиться
        return false;
    }
    if (static_cast<quint32>(buffer.size()) < 4 + len)
        return false;

    const QByteArray payload = buffer.mid(4, static_cast<int>(len));
    buffer.remove(0, static_cast<int>(4 + len));

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    out = doc.object();
    return true;
}
}  // namespace protocol