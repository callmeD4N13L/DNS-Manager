#include "models/NetworkAdapter.hpp"

QString NetworkAdapter::typeToString(Type type)
{
    switch (type) {
    case Type::Ethernet:
        return QStringLiteral("Ethernet");
    case Type::Wifi:
        return QStringLiteral("Wi-Fi");
    case Type::Virtual:
        return QStringLiteral("Virtual");
    case Type::Vpn:
        return QStringLiteral("VPN");
    case Type::Loopback:
        return QStringLiteral("Loopback");
    case Type::Other:
        break;
    }
    return QStringLiteral("Other");
}
