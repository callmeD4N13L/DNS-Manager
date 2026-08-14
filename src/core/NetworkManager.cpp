#include "core/NetworkManager.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QUuid>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shellapi.h>
#endif

#include "platform/WindowsDns.hpp"

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent)
{
}

OperationResult NetworkManager::refresh()
{
    const AdapterListResult result = WindowsDns::enumerateAdapters();
    m_adapters = std::move(result.adapters);

    OperationResult op;
    op.success = result.success;
    op.message = result.message;
    op.errorCode = result.errorCode;

    if (op.success) {
        if (m_selectedId.isEmpty() || adapterById(m_selectedId) == nullptr)
            selectActiveAdapter();
        if (!m_selectedId.isEmpty())
            readCurrentDns();
    }
    emit adaptersChanged();
    return op;
}

OperationResult NetworkManager::readCurrentDns()
{
    const NetworkAdapter* adapter = selectedAdapter();
    if (adapter == nullptr)
        return OperationResult::ok();

    m_currentDns = WindowsDns::readDns(adapter->id);
    emit currentDnsChanged();
    return { m_currentDns.success, m_currentDns.message, m_currentDns.errorCode };
}

const DnsReadResult& NetworkManager::currentDns() const
{
    return m_currentDns;
}

#if defined(Q_OS_WIN)

namespace {

constexpr int kElevatedTimeoutMs = 30000;

bool writeDnsRequest(const QString& path, const QUuid& interfaceGuid, const QStringList& servers)
{
    QJsonObject root;
    root.insert(QStringLiteral("adapter_id"), interfaceGuid.toString(QUuid::WithoutBraces));

    QJsonArray array;
    for (const QString& server : servers)
        array.append(server);
    root.insert(QStringLiteral("servers"), array);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return true;
}

} // namespace

OperationResult NetworkManager::applyDns(const QString& adapterId, const QStringList& servers)
{
    const NetworkAdapter* adapter = adapterById(adapterId);
    if (adapter == nullptr)
        return OperationResult::fail(QObject::tr("Network adapter is no longer available"));

    const QString requestPath = QDir::temp().filePath(
        QStringLiteral("dnsmgr-apply-%1.json")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
    if (!writeDnsRequest(requestPath, adapter->id, servers))
        return OperationResult::fail(QObject::tr("Could not prepare the apply request"));

    const QString params =
        QStringLiteral("\"--apply-dns\" \"%1\"").arg(requestPath);
    const HINSTANCE handle = ShellExecuteW(
        nullptr, L"runas",
        reinterpret_cast<const wchar_t*>(QCoreApplication::applicationFilePath().utf16()),
        reinterpret_cast<const wchar_t*>(params.utf16()),
        nullptr, SW_SHOWNORMAL);
    const intptr_t code = reinterpret_cast<intptr_t>(handle);
    if (code <= 32) {
        QFile::remove(requestPath);
        if (code == SE_ERR_ACCESSDENIED) // user dismissed the UAC prompt
            return OperationResult::fail(
                QObject::tr("Administrator approval was declined"), static_cast<int>(code));
        return OperationResult::fail(
            QObject::tr("Could not start the elevated helper (%1)").arg(code), static_cast<int>(code));
    }

    // Wait for the elevated instance to write the result file. The event loop
    // is pumped so the UI stays responsive while the user approves UAC.
    const QString resultPath = requestPath + QStringLiteral(".result");
    QElapsedTimer timer;
    timer.start();
    QJsonObject result;
    bool completed = false;
    while (timer.elapsed() < kElevatedTimeoutMs) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        if (QFile::exists(resultPath)) {
            QFile resultFile(resultPath);
            if (resultFile.open(QIODevice::ReadOnly)) {
                const QJsonDocument doc = QJsonDocument::fromJson(resultFile.readAll());
                if (doc.isObject()) {
                    result = doc.object();
                    completed = true;
                }
            }
            break;
        }
        QThread::msleep(120);
    }
    QFile::remove(requestPath);
    QFile::remove(resultPath);

    if (!completed)
        return OperationResult::fail(QObject::tr("The elevated operation did not complete in time"));

    OperationResult applied;
    applied.success = result.value(QStringLiteral("success")).toBool();
    applied.message = result.value(QStringLiteral("message")).toString();
    applied.errorCode = result.value(QStringLiteral("error_code")).toInt();
    if (!applied.success)
        return applied;

    // Re-read the real system state so the UI reflects what actually happened.
    readCurrentDns();
    return applied;
}

OperationResult NetworkManager::resetDns(const QString& adapterId)
{
    return applyDns(adapterId, {});
}

OperationResult NetworkManager::flushDnsCache()
{
    return WindowsDns::flushDnsCache();
}

#else // !Q_OS_WIN

OperationResult NetworkManager::applyDns(const QString&, const QStringList&)
{
    return OperationResult::fail(QObject::tr("Changing DNS is not supported on this platform"));
}

OperationResult NetworkManager::resetDns(const QString& adapterId)
{
    return applyDns(adapterId, {});
}

OperationResult NetworkManager::flushDnsCache()
{
    return OperationResult::fail(QObject::tr("Flushing the DNS cache is not supported on this platform"));
}

#endif

const std::vector<NetworkAdapter>& NetworkManager::adapters() const
{
    return m_adapters;
}

const NetworkAdapter* NetworkManager::adapterById(const QString& id) const
{
    const int index = indexOf(id);
    return index >= 0 ? &m_adapters.at(index) : nullptr;
}

const NetworkAdapter* NetworkManager::selectedAdapter() const
{
    return m_selectedId.isEmpty() ? nullptr : adapterById(m_selectedId);
}

QString NetworkManager::selectedAdapterId() const
{
    return m_selectedId;
}

void NetworkManager::setSelectedAdapter(const QString& id)
{
    if (id == m_selectedId)
        return;
    if (adapterById(id) == nullptr)
        return;
    m_selectedId = id;
    emit selectedAdapterChanged();
    readCurrentDns();
}

void NetworkManager::selectActiveAdapter()
{
    const NetworkAdapter* fallback = nullptr;
    for (const NetworkAdapter& adapter : m_adapters) {
        if (adapter.connected) {
            setSelectedAdapter(adapter.id.toString(QUuid::WithoutBraces));
            return;
        }
        if (fallback == nullptr && adapter.enabled)
            fallback = &adapter;
    }
    if (fallback != nullptr)
        setSelectedAdapter(fallback->id.toString(QUuid::WithoutBraces));
    else if (!m_adapters.empty())
        setSelectedAdapter(m_adapters.front().id.toString(QUuid::WithoutBraces));
}

int NetworkManager::indexOf(const QString& id) const
{
    for (int i = 0; i < static_cast<int>(m_adapters.size()); ++i) {
        if (m_adapters.at(i).id.toString(QUuid::WithoutBraces) == id)
            return i;
    }
    return -1;
}
