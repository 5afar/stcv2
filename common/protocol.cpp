#include "common/protocol.h"

#include <QJsonDocument>
#include <QJsonParseError>

namespace protocol {
QByteArray encode(const QJsonObject& obj) {
    const QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = static_cast<quint32>(json.size());

    QByteArray frame;
    frame.resize(4);
    frame[0] = static_cast<char>((len >> 24) & 0xFF);
    frame[1] = static_cast<char>((len >> 16) & 0xFF);
    frame[2] = static_cast<char>((len >> 8) & 0xFF);
    frame[3] = static_cast<char>(len & 0xFF);
    frame.append(json);
    return frame;
}

bool tryExtract(QByteArray& buffer, QJsonObject& out) {
    if (buffer.size() < 4)
        return false;

    const quint32 len = (static_cast<quint32>(static_cast<quint8>(buffer[0])) << 24) |
                        (static_cast<quint32>(static_cast<quint8>(buffer[1])) << 16) |
                        (static_cast<quint32>(static_cast<quint8>(buffer[2])) << 8) |
                        (static_cast<quint32>(static_cast<quint8>(buffer[3])));

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