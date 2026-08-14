#pragma once

#include <QString>
#include <QStringList>
#include <QUuid>
#include <vector>

#include "core/OperationResult.hpp"
#include "models/DnsConfiguration.hpp"
#include "models/NetworkAdapter.hpp"

// Result of adapter enumeration: OperationResult + the adapter list.
struct AdapterListResult : OperationResult
{
    std::vector<NetworkAdapter> adapters;
};

// Current DNS configuration of one adapter.
struct DnsReadResult : OperationResult
{
    DnsConfiguration configuration;
    bool isDhcp = true; // true when the adapter uses automatic (DHCP) DNS
};

// Thin wrapper around the Windows IP Helper API.
// All functions return structured results; they never throw across the API
// boundary and never silently ignore failures.
namespace WindowsDns {

AdapterListResult enumerateAdapters();

// Reads the currently configured DNS of an adapter identified by its
// interface GUID. Prefers GetInterfaceDnsSettings (accurate static/DHCP
// distinction) and falls back to GetAdaptersAddresses server list.
DnsReadResult readDns(const QUuid& interfaceGuid);

// Applies a DNS server list to the adapter (empty list = revert to DHCP).
// Requires administrator privileges (the elevated helper calls this).
OperationResult setDns(const QUuid& interfaceGuid, const QStringList& servers);

// Reads a JSON request file written by the GUI and applies it. Used by the
// elevated instance started with --apply-dns. The request shape:
//   { "adapter_id": "<guid>", "servers": ["1.1.1.1", "..."] }
// The caller writes "<requestPath>.result" with the OperationResult fields.
OperationResult applyDnsFromFile(const QString& requestPath);

// Maps a Win32 error code to a human-readable message.
QString errorMessage(int errorCode);

// Flushes the DNS resolver cache (like ipconfig /flushdns). Does not require
// administrator privileges.
OperationResult flushDnsCache();

} // namespace WindowsDns