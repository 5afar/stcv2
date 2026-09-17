#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QJsonObject>

namespace protocol {

/// 2 в 20 байт под максимальный размер сообщения
constexpr quint32 kMaxMessageSize = 1u << 20;

QByteArray encode(const QJsonObject& obj);

bool tryExtract(QByteArray& buffer, QJsonObject& out);
}  // namespace protocol
#endif  // PROTOCOL_H
