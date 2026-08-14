#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <windns.h>
#include <ws2tcpip.h>

#include <QCryptographicHash>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QUuid>
#include <memory>
#include <string>

#include "platform/WindowsDns.hpp"

namespace {

constexpr ULONG kAdapterFlags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;

bool isVirtualName(const QString& friendlyName)
{
    const QString lower = friendlyName.toCaseFolded();
    return lower.contains(QStringLiteral("virtual"))
        || lower.contains(QStringLiteral("vethernet"))
        || lower.contains(QStringLiteral("hyper-v"));
}

NetworkAdapter::Type mapAdapterType(ULONG ifType, const QString& friendlyName)
{
    switch (ifType) {
    case IF_TYPE_SOFTWARE_LOOPBACK:
        return NetworkAdapter::Type::Loopback;
    case IF_TYPE_IEEE80211:
        return NetworkAdapter::Type::Wifi;
    case IF_TYPE_PPP:
        return NetworkAdapter::Type::Vpn;
    case IF_TYPE_TUNNEL:
        return NetworkAdapter::Type::Vpn;
    case IF_TYPE_ETHERNET_CSMACD:
        return isVirtualName(friendlyName) ? NetworkAdapter::Type::Virtual
                                           : NetworkAdapter::Type::Ethernet;
    default:
        return isVirtualName(friendlyName) ? NetworkAdapter::Type::Virtual
                                           : NetworkAdapter::Type::Other;
    }
}

QString ipToString(const SOCKADDR* sockaddr)
{
    if (sockaddr == nullptr)
        return {};

    char buffer[INET6_ADDRSTRLEN] = {};
    if (sockaddr->sa_family == AF_INET) {
        const auto* v4 = reinterpret_cast<const sockaddr_in*>(sockaddr);
        if (inet_ntop(AF_INET, &v4->sin_addr, buffer, sizeof(buffer)))
            return QString::fromLatin1(buffer);
    } else if (sockaddr->sa_family == AF_INET6) {
        const auto* v6 = reinterpret_cast<const sockaddr_in6*>(sockaddr);
        if (inet_ntop(AF_INET6, &v6->sin6_addr, buffer, sizeof(buffer)))
            return QString::fromLatin1(buffer);
    }
    return {};
}

QUuid guidFromWin32(const GUID& guid)
{
    return QUuid(guid.Data1, guid.Data2, guid.Data3,
                 guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                 guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

// Deterministic fallback id (LUID->GUID conversion is not always available).
QUuid stableIdFromName(const QString& name)
{
    const QByteArray md5 = QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Md5);
    const auto* d = reinterpret_cast<const uchar*>(md5.constData());
    return QUuid((d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3],
                 quint16((d[4] << 8) | d[5]),
                 quint16((d[6] << 8) | d[7]),
                 d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
}

QUuid adapterId(const NET_LUID& luid, const QString& name)
{
    GUID guid;
    if (ConvertInterfaceLuidToGuid(&luid, &guid) == NO_ERROR)
        return guidFromWin32(guid);
    return stableIdFromName(name);
}

// GetAdaptersInfo is legacy but the simplest reliable source of the IPv4
// DHCP flag, keyed by the adapter interface index.
QHash<quint32, bool> queryDhcpFlags()
{
    QHash<quint32, bool> flags;
    ULONG size = 0;
    if (GetAdaptersInfo(nullptr, &size) != ERROR_BUFFER_OVERFLOW || size == 0)
        return flags;

    std::unique_ptr<void, decltype(&free)> buffer(std::malloc(size), &free);
    if (!buffer)
        return flags;

    auto* info = static_cast<PIP_ADAPTER_INFO>(buffer.get());
    if (GetAdaptersInfo(info, &size) != NO_ERROR)
        return flags;

    for (PIP_ADAPTER_INFO entry = info; entry; entry = entry->Next)
        flags.insert(entry->Index, entry->DhcpEnabled == TRUE);
    return flags;
}

GUID guidFromQUuid(const QUuid& uuid)
{
    GUID guid;
    guid.Data1 = uuid.data1();
    guid.Data2 = uuid.data2();
    guid.Data3 = uuid.data3();
    for (int i = 0; i < 8; ++i)
        guid.Data4[i] = uuid.data4()[i];
    return guid;
}

// "1.1.1.1,1.0.0.1" -> primary/secondary per family.
DnsConfiguration configurationFromServerList(const QStringList& servers)
{
    QStringList ipv4;
    QStringList ipv6;
    for (const QString& raw : servers) {
        const QString server = raw.trimmed();
        if (Dns::isValidIpv4(server))
            ipv4.append(server);
        else if (Dns::isValidIpv6(server))
            ipv6.append(server);
    }

    DnsConfiguration configuration;
    if (!ipv4.isEmpty())
        configuration.primaryIpv4 = ipv4.at(0);
    if (ipv4.size() > 1)
        configuration.secondaryIpv4 = ipv4.at(1);
    if (!ipv6.isEmpty())
        configuration.primaryIpv6 = ipv6.at(0);
    if (ipv6.size() > 1)
        configuration.secondaryIpv6 = ipv6.at(1);
    return configuration;
}

} // namespace

namespace WindowsDns {

QString errorMessage(int errorCode)
{
    wchar_t* buffer = nullptr;
    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, static_cast<DWORD>(errorCode), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

    QString message;
    if (length > 0 && buffer != nullptr) {
        message = QString::fromWCharArray(buffer, static_cast<int>(length)).trimmed();
        LocalFree(buffer);
    }
    if (message.isEmpty())
        message = QObject::tr("Windows error %1").arg(errorCode);
    return message;
}

AdapterListResult enumerateAdapters()
{
    ULONG bufferLength = 0;
    DWORD ret = GetAdaptersAddresses(AF_UNSPEC, kAdapterFlags, nullptr, nullptr, &bufferLength);
    if (ret == ERROR_NO_DATA) // no adapters at all
        return AdapterListResult { OperationResult::ok(), {} };
    if (ret != NO_ERROR && ret != ERROR_BUFFER_OVERFLOW)
        return AdapterListResult { OperationResult::fail(errorMessage(ret), static_cast<int>(ret)), {} };
    if (bufferLength == 0)
        bufferLength = sizeof(IP_ADAPTER_ADDRESSES);

    std::unique_ptr<void, decltype(&free)> buffer(std::malloc(bufferLength), &free);
    if (!buffer)
        return AdapterListResult { OperationResult::fail(QStringLiteral("Out of memory")), {} };

    auto* first = static_cast<PIP_ADAPTER_ADDRESSES>(buffer.get());
    ret = GetAdaptersAddresses(AF_UNSPEC, kAdapterFlags, nullptr, first, &bufferLength);
    if (ret != NO_ERROR)
        return AdapterListResult { OperationResult::fail(errorMessage(ret), static_cast<int>(ret)), {} };

    const QHash<quint32, bool> dhcpFlags = queryDhcpFlags();

    std::vector<NetworkAdapter> adapters;
    for (PIP_ADAPTER_ADDRESSES entry = first; entry; entry = entry->Next) {
        NetworkAdapter adapter;
        adapter.name = QString::fromLatin1(entry->AdapterName);
        adapter.friendlyName = QString::fromWCharArray(entry->FriendlyName);
        adapter.type = mapAdapterType(entry->IfType, adapter.friendlyName);
        adapter.enabled = entry->OperStatus != IfOperStatusDown;
        adapter.connected = entry->OperStatus == IfOperStatusUp;
        adapter.dhcpEnabled = dhcpFlags.value(entry->IfIndex, false);
        adapter.id = adapterId(entry->Luid, adapter.name);

        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = entry->FirstUnicastAddress;
             unicast; unicast = unicast->Next) {
            const QString ip = ipToString(unicast->Address.lpSockaddr);
            if (ip.isEmpty())
                continue;
            if (unicast->Address.lpSockaddr->sa_family == AF_INET)
                adapter.ipv4Addresses.append(ip);
            else if (unicast->Address.lpSockaddr->sa_family == AF_INET6)
                adapter.ipv6Addresses.append(ip);
        }

        for (PIP_ADAPTER_DNS_SERVER_ADDRESS dns = entry->FirstDnsServerAddress;
             dns; dns = dns->Next) {
            const QString ip = ipToString(dns->Address.lpSockaddr);
            if (!ip.isEmpty())
                adapter.dnsServers.append(ip);
        }

        adapters.push_back(std::move(adapter));
    }

    return AdapterListResult { OperationResult::ok(), std::move(adapters) };
}

DnsReadResult readDns(const QUuid& interfaceGuid)
{
    const GUID guid = guidFromQUuid(interfaceGuid);

    // Preferred path: GetInterfaceDnsSettings reports the name servers plus a
    // Flags field that says whether the list is static (0) or came from DHCP.
    NET_LUID luid;
    if (ConvertInterfaceGuidToLuid(&guid, &luid) == NO_ERROR) {
        DNS_INTERFACE_SETTINGS settings;
        RtlZeroMemory(&settings, sizeof(settings));
        settings.Version = DNS_INTERFACE_SETTINGS_VERSION1;

        const DWORD ret = GetInterfaceDnsSettings(DnsInterfaceTypeUnspecified, &luid, &settings);
        if (ret == NO_ERROR) {
            const bool staticDns = (settings.Flags & DNS_INTERFACE_SETTINGS_STATIC_DNS) != 0;
            QStringList servers;
            if (settings.NameServer != nullptr)
                servers = QString::fromWCharArray(settings.NameServer).split(QLatin1Char(','));
            DnsReadResult result;
            result.configuration = configurationFromServerList(servers);
            result.isDhcp = !staticDns;
            result.success = true;
            return result;
        }
        // Fall through to GetAdaptersAddresses on error (feature unsupported,
        // interface gone, access denied, etc.).
    }

    // Fallback: enumerate the adapter and collect its configured DNS servers.
    // The adapter list is keyed by the same GUID-derived id.
    AdapterListResult enumeration = enumerateAdapters();
    if (!enumeration.success)
        return DnsReadResult { enumeration, {}, true };

    for (const NetworkAdapter& adapter : enumeration.adapters) {
        if (adapter.id != interfaceGuid)
            continue;

        DnsReadResult result;
        result.configuration = configurationFromServerList(adapter.dnsServers);
        // On this fallback path the static/DHCP distinction is unreliable;
        // report DHCP only when the adapter flag says so and there are no
        // configured servers.
        result.isDhcp = adapter.dhcpEnabled && adapter.dnsServers.isEmpty();
        result.success = true;
        return result;
    }

    return DnsReadResult { OperationResult::fail(QObject::tr("Adapter not found")), {}, true };
}

OperationResult setDns(const QUuid& interfaceGuid, const QStringList& servers)
{
    const GUID guid = guidFromQUuid(interfaceGuid);

    NET_LUID luid;
    if (ConvertInterfaceGuidToLuid(&guid, &luid) != NO_ERROR)
        return OperationResult::fail(QObject::tr("Network interface not found"));

    DNS_INTERFACE_SETTINGS settings;
    RtlZeroMemory(&settings, sizeof(settings));
    settings.Version = DNS_INTERFACE_SETTINGS_VERSION1;

    std::wstring nameServers;
    if (servers.isEmpty()) {
        // Empty list = give DNS back to DHCP.
        settings.Flags = DNS_INTERFACE_SETTINGS_RESET_DHCP;
        settings.NameServer = nullptr;
    } else {
        for (int i = 0; i < servers.size(); ++i) {
            if (i > 0)
                nameServers += L',';
            nameServers += servers.at(i).toStdWString();
        }
        settings.Flags = DNS_INTERFACE_SETTINGS_NAME_SERVER
            | DNS_INTERFACE_SETTINGS_IPV4 | DNS_INTERFACE_SETTINGS_IPV6;
        settings.NameServer = nameServers.data();
    }

    const DWORD ret = SetInterfaceDnsSettings(&luid, &settings);
    if (ret != NO_ERROR)
        return OperationResult::fail(errorMessage(ret), static_cast<int>(ret));
    return OperationResult::ok();
}

OperationResult applyDnsFromFile(const QString& requestPath)
{
    QFile file(requestPath);
    if (!file.open(QIODevice::ReadOnly))
        return OperationResult::fail(QObject::tr("Could not read request file"));

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (doc.isNull() || parseError.error != QJsonParseError::NoError)
        return OperationResult::fail(QObject::tr("Request file is not valid JSON"));

    const QJsonObject root = doc.object();
    const QUuid interfaceGuid(root.value(QStringLiteral("adapter_id")).toString());
    if (interfaceGuid.isNull())
        return OperationResult::fail(QObject::tr("Request has no valid adapter id"));

    QStringList servers;
    const QJsonArray array = root.value(QStringLiteral("servers")).toArray();
    servers.reserve(array.size());
    for (const QJsonValue& value : array)
        servers.append(value.toString());

    return setDns(interfaceGuid, servers);
}

OperationResult flushDnsCache()
{
    if (DnsFlushResolverCache() == TRUE)
        return OperationResult::ok();
    const DWORD error = GetLastError();
    return OperationResult::fail(errorMessage(error), static_cast<int>(error));
}

} // namespace WindowsDns
