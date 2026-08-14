#pragma once

#include <QObject>
#include <QString>
#include <vector>

#include "core/OperationResult.hpp"
#include "models/NetworkAdapter.hpp"
#include "platform/WindowsDns.hpp"

// High-level network service: discovers adapters and tracks which one the
// user wants to modify. Uses WindowsDns (platform layer) for the actual
// system calls; later phases add read/write DNS + reset here.
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);

    OperationResult refresh();
    OperationResult readCurrentDns();

    // Applies DNS servers to the adapter (empty list = revert to DHCP).
    // Requires administrator rights: this runs an elevated copy of the app
    // (--apply-dns) via ShellExecute "runas" and waits for its result file.
    // UI stays responsive (event loop is pumped while polling).
    OperationResult applyDns(const QString& adapterId, const QStringList& servers);
    OperationResult resetDns(const QString& adapterId);

    // Flushes the system DNS resolver cache (no elevation needed).
    OperationResult flushDnsCache();

    const std::vector<NetworkAdapter>& adapters() const;
    const NetworkAdapter* adapterById(const QString& id) const;
    const NetworkAdapter* selectedAdapter() const;

    QString selectedAdapterId() const;
    void setSelectedAdapter(const QString& id);

    // Current DNS configuration of the selected adapter.
    const DnsReadResult& currentDns() const;

signals:
    void adaptersChanged();
    void selectedAdapterChanged();
    void currentDnsChanged();

private:
    void selectActiveAdapter();
    int indexOf(const QString& id) const;

    std::vector<NetworkAdapter> m_adapters;
    QString m_selectedId;
    DnsReadResult m_currentDns;
};
