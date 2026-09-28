// ---------------------------------------------------------------------------
// dns-core: headless JSON-RPC sidecar for the Electron/React UI.
//
// Reuses the existing C++ backend (core/, models/, platform/) with NO
// business-logic duplication. Speaks newline-delimited JSON (NDJSON) over
// stdio:
//
//   stdin:  {"id": 1, "method": "refresh", "params": {...}}
//   stdout: {"id": 1, "ok": true, "result": {...}}
//   stdout: {"id": 2, "ok": false, "error": {"message": "...", "code": 0}}
//
// Async notifications (never carry an "id"):
//
//   stdout: {"event": "notification", "payload": {"text": "...", "kind": "info"}}
//
// Every method maps 1:1 to the application's backend API (NetworkManager,
// ProfileManager, AppSettings, DnsLatencyTester), so the Electron UI exposes
// exactly the same functionality — no invented features, no mock logic.
//
// Methods:
//   getState            full snapshot (meta + adapters + currentDns +
//                       profiles + settings + activeProfileId)
//   refresh             re-detect adapters
//   selectAdapter       {id}
//   applyProfile        {id}            (UAC elevation as before)
//   resetDns            {}              (operates on selected adapter)
//   flushCache          {}
//   benchmark           {}              (sync latency probe of selected adapter)
//   profiles.list       {search?}
//   profiles.add        {name, provider?, description?, primaryIpv4?, ...}
//   profiles.update     {id, ...fields}
//   profiles.remove     {id}
//   profiles.toggleFavorite {id}
//   profiles.import     {path}          (path validated: must exist, .json)
//   profiles.export     {path}          (parent dir must exist)
//   profiles.reset      {}
//   settings.get        {}
//   settings.update     {themeMode?, startWithWindows?, ...}
//   settings.reset      {}
//
// Validation: profile field validation reuses DnsProfile::isValid() via
// ProfileManager::addProfile/updateProfile (same code path as the UI).
// Path arguments are canonicalised and restricted to *.json files.
// ---------------------------------------------------------------------------

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QTimer>
#include <QVariantMap>
#include <QtGlobal>

#if defined(Q_OS_WIN)
#include <QDir>
#endif

#include "core/AppSettings.hpp"
#include "core/DnsLatencyTester.hpp"
#include "core/NetworkManager.hpp"
#include "core/OperationResult.hpp"
#include "core/ProfileManager.hpp"
#include "core/ProfileStorage.hpp"
#include "models/DnsProfile.hpp"
#include "models/NetworkAdapter.hpp"
#include "platform/WindowsDns.hpp"

// Version injected by CMake (DNSMGR_VERSION="${PROJECT_VERSION}"); the literal
// below is only a fallback for ad-hoc single-file builds that bypass CMake.
#ifndef DNSMGR_VERSION
#define DNSMGR_VERSION "2.0.0"
#endif

namespace {

// ---------------------------------------------------------------------------
// Serialization helpers
// ---------------------------------------------------------------------------

QJsonObject opToJson(const OperationResult& op)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("success"), op.success);
    obj.insert(QStringLiteral("message"), op.message);
    obj.insert(QStringLiteral("errorCode"), op.errorCode);
    return obj;
}

QJsonObject adapterToJson(const NetworkAdapter& adapter, const QString& selectedId)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), adapter.id.toString(QUuid::WithoutBraces));
    obj.insert(QStringLiteral("name"), adapter.name);
    obj.insert(QStringLiteral("friendlyName"), adapter.friendlyName);
    obj.insert(QStringLiteral("type"), NetworkAdapter::typeToString(adapter.type));
    obj.insert(QStringLiteral("typeLabel"), NetworkAdapter::typeToString(adapter.type));
    obj.insert(QStringLiteral("enabled"), adapter.enabled);
    obj.insert(QStringLiteral("connected"), adapter.connected);
    obj.insert(QStringLiteral("dhcpEnabled"), adapter.dhcpEnabled);
    QJsonArray v4;
    for (const QString& address : adapter.ipv4Addresses)
        v4.append(address);
    obj.insert(QStringLiteral("ipv4"), v4);
    QJsonArray v6;
    for (const QString& address : adapter.ipv6Addresses)
        v6.append(address);
    obj.insert(QStringLiteral("ipv6"), v6);
    QJsonArray dns;
    for (const QString& server : adapter.dnsServers)
        dns.append(server);
    obj.insert(QStringLiteral("dnsServers"), dns);
    obj.insert(QStringLiteral("selected"),
               adapter.id.toString(QUuid::WithoutBraces) == selectedId);
    return obj;
}

QJsonObject profileToJson(const DnsProfile& profile)
{
    QJsonObject obj = profile.toJson();
    // Camel-case aliases for the TypeScript renderer (snake_case stays
    // canonical for import/export file compatibility).
    obj.insert(QStringLiteral("primaryIpv4"), profile.primaryIpv4);
    obj.insert(QStringLiteral("secondaryIpv4"), profile.secondaryIpv4);
    obj.insert(QStringLiteral("primaryIpv6"), profile.primaryIpv6);
    obj.insert(QStringLiteral("secondaryIpv6"), profile.secondaryIpv6);
    obj.insert(QStringLiteral("isFavorite"), profile.isFavorite);
    obj.insert(QStringLiteral("summary"), profile.summary());
    return obj;
}

QJsonObject settingsToJson(AppSettings* settings)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("themeMode"), settings->themeMode());
    obj.insert(QStringLiteral("startWithWindows"), settings->startWithWindows());
    obj.insert(QStringLiteral("startMinimized"), settings->startMinimized());
    obj.insert(QStringLiteral("minimizeToTray"), settings->minimizeToTray());
    obj.insert(QStringLiteral("flushCacheAfterApply"), settings->flushCacheAfterApply());
    obj.insert(QStringLiteral("confirmBeforeApply"), settings->confirmBeforeApply());
    obj.insert(QStringLiteral("settingsFilePath"), settings->settingsFilePath());
    return obj;
}

// ---------------------------------------------------------------------------
// Service: owns the backend managers (same composition as Backend).
// ---------------------------------------------------------------------------

class CoreService
{
public:
    CoreService()
        : m_profiles(new ProfileManager)
        , m_network(new NetworkManager)
        , m_settings(new AppSettings)
        , m_tester(new DnsLatencyTester)
    {
        m_profiles->load();
        m_network->refresh();
    }

    ~CoreService()
    {
        delete m_tester;
        delete m_settings;
        delete m_network;
        delete m_profiles;
    }

    QJsonObject meta() const
    {
        QJsonObject obj;
        obj.insert(QStringLiteral("version"), QCoreApplication::applicationVersion());
#if defined(Q_OS_WIN)
        obj.insert(QStringLiteral("platform"), QStringLiteral("Windows"));
#else
        obj.insert(QStringLiteral("platform"), QStringLiteral("Unknown"));
#endif
        obj.insert(QStringLiteral("qtVersion"), QString::fromUtf8(qVersion()));
        obj.insert(QStringLiteral("profilesFilePath"), ProfileStorage::filePath());
        return obj;
    }

    QJsonArray adaptersJson() const
    {
        QJsonArray array;
        const QString selected = m_network->selectedAdapterId();
        for (const NetworkAdapter& adapter : m_network->adapters())
            array.append(adapterToJson(adapter, selected));
        return array;
    }

    QJsonObject currentDnsJson() const
    {
        const DnsReadResult& current = m_network->currentDns();
        const NetworkAdapter* adapter = m_network->selectedAdapter();
        QJsonObject obj = opToJson({ current.success, current.message, current.errorCode });
        obj.insert(QStringLiteral("adapterId"), m_network->selectedAdapterId());
        obj.insert(QStringLiteral("adapterName"),
                   adapter != nullptr ? adapter->friendlyName : QString());
        obj.insert(QStringLiteral("connected"),
                   adapter != nullptr && adapter->enabled && adapter->connected);
        obj.insert(QStringLiteral("primaryIpv4"), current.configuration.primaryIpv4);
        obj.insert(QStringLiteral("secondaryIpv4"), current.configuration.secondaryIpv4);
        obj.insert(QStringLiteral("primaryIpv6"), current.configuration.primaryIpv6);
        obj.insert(QStringLiteral("secondaryIpv6"), current.configuration.secondaryIpv6);
        obj.insert(QStringLiteral("isDhcp"), current.isDhcp);
        obj.insert(QStringLiteral("available"),
                   adapter != nullptr && current.success);
        return obj;
    }

    QJsonArray profilesJson(const QString& search = {}) const
    {
        QJsonArray array;
        const QString needle = search.trimmed().toLower();
        for (const DnsProfile& profile : m_profiles->profiles()) {
            if (!needle.isEmpty()) {
                const QString haystack =
                    (profile.name + QLatin1Char(' ') + profile.provider + QLatin1Char(' ')
                     + profile.summary())
                        .toLower();
                if (!haystack.contains(needle))
                    continue;
            }
            array.append(profileToJson(profile));
        }
        return array;
    }

    QJsonObject fullState(const QString& search = {}) const
    {
        QJsonObject state;
        state.insert(QStringLiteral("meta"), meta());
        state.insert(QStringLiteral("adapters"), adaptersJson());
        state.insert(QStringLiteral("selectedAdapterId"), m_network->selectedAdapterId());
        state.insert(QStringLiteral("currentDns"), currentDnsJson());
        state.insert(QStringLiteral("profiles"), profilesJson(search));
        state.insert(QStringLiteral("settings"), settingsToJson(m_settings));
        return state;
    }

    // Runs one latency probe per DNS server of the selected adapter,
    // sequentially, reusing DnsLatencyTester (the real UDP probe algorithm).
    QJsonObject benchmark()
    {
        QJsonObject result;
        const NetworkAdapter* adapter = m_network->selectedAdapter();
        if (adapter == nullptr) {
            result.insert(QStringLiteral("latencyMs"), -1);
            result.insert(QStringLiteral("noticeText"),
                          QStringLiteral("No network adapter selected"));
            result.insert(QStringLiteral("noticeKind"), QStringLiteral("error"));
            return result;
        }
        if (adapter->dnsServers.isEmpty()) {
            result.insert(QStringLiteral("latencyMs"), -1);
            result.insert(QStringLiteral("noticeText"),
                          QStringLiteral("No DNS servers configured on this adapter"));
            result.insert(QStringLiteral("noticeKind"), QStringLiteral("info"));
            return result;
        }

        QJsonArray details;
        double best = -1;
        QString bestServer;
        for (const QString& server : adapter->dnsServers) {
            QString gotServer;
            double latency = -1;
            bool ok = false;
            QString message;
            QEventLoop loop;
            QTimer timeout;
            timeout.setSingleShot(true);
            timeout.setInterval(8000);
            QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
            QObject::connect(
                m_tester, &DnsLatencyTester::testFinished, &loop,
                [&](const QString& srv, double ms, bool success, const QString& msg) {
                    if (srv != server)
                        return; // stale signal from an earlier probe
                    gotServer = srv;
                    latency = ms;
                    ok = success;
                    message = msg;
                    loop.quit();
                });
            timeout.start();
            m_tester->test(server);
            loop.exec();
            // Disconnect the per-probe handler before the next iteration.
            QObject::disconnect(m_tester, nullptr, &loop, nullptr);

            QJsonObject entry;
            entry.insert(QStringLiteral("server"), server);
            entry.insert(QStringLiteral("ok"), ok);
            entry.insert(QStringLiteral("latencyMs"), ok ? qRound(latency) : -1);
            entry.insert(QStringLiteral("message"), message);
            details.append(entry);
            if (ok && (best < 0 || latency < best)) {
                best = latency;
                bestServer = server;
            }
        }

        result.insert(QStringLiteral("results"), details);
        result.insert(QStringLiteral("latencyMs"), best < 0 ? -1 : qRound(best));
        result.insert(QStringLiteral("bestServer"), bestServer);
        if (best < 0) {
            result.insert(QStringLiteral("noticeText"),
                          QStringLiteral("All DNS servers timed out"));
            result.insert(QStringLiteral("noticeKind"), QStringLiteral("error"));
        } else {
            result.insert(
                QStringLiteral("noticeText"),
                QStringLiteral("Fastest DNS latency: %1 ms via %2")
                    .arg(qRound(best))
                    .arg(bestServer));
            result.insert(QStringLiteral("noticeKind"), QStringLiteral("success"));
        }
        return result;
    }

    // Converts camelCase renderer params to the QVariantMap shape
    // ProfileManager expects (same keys as the renderer ProfileDialog sends).
    static QVariantMap toProfileFields(const QJsonObject& params)
    {
        QVariantMap fields;
        const auto copy = [&](const char* jsonKey, const char* fieldKey) {
            if (params.contains(QLatin1String(jsonKey)))
                fields.insert(QLatin1String(fieldKey),
                              params.value(QLatin1String(jsonKey)).toString());
        };
        copy("name", "name");
        copy("provider", "provider");
        copy("description", "description");
        copy("primaryIpv4", "primaryIpv4");
        copy("secondaryIpv4", "secondaryIpv4");
        copy("primaryIpv6", "primaryIpv6");
        copy("secondaryIpv6", "secondaryIpv6");
        if (params.contains(QStringLiteral("isFavorite")))
            fields.insert(QStringLiteral("favorite"),
                          params.value(QStringLiteral("isFavorite")).toBool());
        else if (params.contains(QStringLiteral("favorite")))
            fields.insert(QStringLiteral("favorite"),
                          params.value(QStringLiteral("favorite")).toBool());
        // Accept snake_case too (import payloads / hand-written JSON).
        copy("primary_ipv4", "primaryIpv4");
        copy("secondary_ipv4", "secondaryIpv4");
        copy("primary_ipv6", "primaryIpv6");
        copy("secondary_ipv6", "secondaryIpv6");
        return fields;
    }

    static bool isSafeJsonPath(const QString& path)
    {
        if (path.isEmpty() || path.size() > 32767)
            return false;
        if (path.contains(QLatin1String("..")))
            return false;
        return path.endsWith(QLatin1String(".json"), Qt::CaseInsensitive);
    }

    ProfileManager* profiles() const { return m_profiles; }
    NetworkManager* network() const { return m_network; }
    AppSettings* settings() const { return m_settings; }

private:
    ProfileManager* m_profiles = nullptr;
    NetworkManager* m_network = nullptr;
    AppSettings* m_settings = nullptr;
    DnsLatencyTester* m_tester = nullptr;
};

// Emits one NDJSON line on stdout.
void emitLine(const QJsonObject& obj)
{
    QTextStream out(stdout, QIODevice::WriteOnly);
    out << QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)) << '\n';
    out.flush();
}

QJsonObject errorEnvelope(const QJsonValue& id, const QString& message, int code = 0)
{
    QJsonObject err;
    err.insert(QStringLiteral("message"), message);
    err.insert(QStringLiteral("code"), code);
    QJsonObject env;
    env.insert(QStringLiteral("id"), id);
    env.insert(QStringLiteral("ok"), false);
    env.insert(QStringLiteral("error"), err);
    return env;
}

QJsonObject okEnvelope(const QJsonValue& id, const QJsonObject& result)
{
    QJsonObject env;
    env.insert(QStringLiteral("id"), id);
    env.insert(QStringLiteral("ok"), true);
    env.insert(QStringLiteral("result"), result);
    return env;
}

void notify(const QString& text, const QString& kind)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("text"), text);
    payload.insert(QStringLiteral("kind"), kind);
    QJsonObject env;
    env.insert(QStringLiteral("event"), QStringLiteral("notification"));
    env.insert(QStringLiteral("payload"), payload);
    emitLine(env);
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("DnsManager"));
    QCoreApplication::setOrganizationName(QStringLiteral("DnsManager"));
    QCoreApplication::setApplicationVersion(QString::fromUtf8(DNSMGR_VERSION));

    const QStringList args = QCoreApplication::arguments();

#if defined(Q_OS_WIN)
    // Elevated helper entry point: applies a DNS request file written by
    // NetworkManager and exits with the outcome.
    const int applyIndex = args.indexOf(QStringLiteral("--apply-dns"));
    if (applyIndex >= 0 && applyIndex + 1 < args.size()) {
        const OperationResult applied =
            WindowsDns::applyDnsFromFile(args.at(applyIndex + 1));
        QJsonObject result;
        result.insert(QStringLiteral("success"), applied.success);
        result.insert(QStringLiteral("message"), applied.message);
        result.insert(QStringLiteral("error_code"), applied.errorCode);
        QFile out(args.at(applyIndex + 1) + QStringLiteral(".result"));
        if (out.open(QIODevice::WriteOnly))
            out.write(QJsonDocument(result).toJson(QJsonDocument::Compact));
        return applied.success ? 0 : 1;
    }
#endif

    if (args.contains(QStringLiteral("--version"))) {
        QTextStream(stdout) << "dns-core "
                            << QCoreApplication::applicationVersion() << '\n';
        return 0;
    }

    CoreService service;
    QTextStream in(stdin, QIODevice::ReadOnly);

    // Readable handshake so the Electron host can detect a live backend.
    {
        QJsonObject hello;
        hello.insert(QStringLiteral("event"), QStringLiteral("ready"));
        hello.insert(QStringLiteral("payload"), service.meta());
        emitLine(hello);
    }

    QString buffer;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty())
            continue;
        buffer = line;

        QJsonParseError parseError{};
        const QJsonDocument doc = QJsonDocument::fromJson(buffer.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emitLine(errorEnvelope(QJsonValue::Null, QStringLiteral("Invalid JSON request")));
            continue;
        }
        const QJsonObject req = doc.object();
        const QJsonValue id = req.value(QStringLiteral("id"));
        const QString method = req.value(QStringLiteral("method")).toString();
        const QJsonObject params =
            req.value(QStringLiteral("params")).toObject();

        // --- dispatch -------------------------------------------------------
        if (method == QStringLiteral("getState")) {
            emitLine(okEnvelope(id, service.fullState(params.value(QStringLiteral("search")).toString())));
        } else if (method == QStringLiteral("refresh")) {
            const OperationResult op = service.network()->refresh();
            QJsonObject result = service.fullState();
            result.insert(QStringLiteral("op"), opToJson(op));
            if (!op.success)
                notify(QStringLiteral("Failed to detect network adapters: %1").arg(op.message),
                       QStringLiteral("error"));
            else if (service.network()->adapters().empty())
                notify(QStringLiteral("No network adapters were detected"), QStringLiteral("warning"));
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("selectAdapter")) {
            const QString adapterId = params.value(QStringLiteral("id")).toString();
            if (adapterId.isEmpty()) {
                emitLine(errorEnvelope(id, QStringLiteral("Missing adapter id")));
                continue;
            }
            service.network()->setSelectedAdapter(adapterId);
            QJsonObject result = service.fullState();
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("applyProfile")) {
            const QString profileId = params.value(QStringLiteral("id")).toString();
            const DnsProfile* profile = service.profiles()->profileById(profileId);
            if (profile == nullptr) {
                emitLine(errorEnvelope(id, QStringLiteral("Profile not found")));
                continue;
            }
            const QString adapterId = service.network()->selectedAdapterId();
            if (adapterId.isEmpty()) {
                emitLine(errorEnvelope(id, QStringLiteral("No network adapter selected")));
                continue;
            }
            const OperationResult op =
                service.network()->applyDns(adapterId, profile->serversList());
            QJsonObject result;
            result.insert(QStringLiteral("op"), opToJson(op));
            if (op.success) {
                if (service.settings()->flushCacheAfterApply())
                    service.network()->flushDnsCache();
                const QString adapterName = service.currentDnsJson()
                                                .value(QStringLiteral("adapterName"))
                                                .toString();
                notify(QStringLiteral("Applied \u201C%1\u201D to %2").arg(profile->name, adapterName),
                       QStringLiteral("success"));
            } else {
                notify(QStringLiteral("Could not apply \u201C%1\u201D: %2")
                           .arg(profile->name, op.message),
                       QStringLiteral("error"));
            }
            result.insert(QStringLiteral("state"), service.fullState());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("resetDns")) {
            const QString adapterId = service.network()->selectedAdapterId();
            if (adapterId.isEmpty()) {
                emitLine(errorEnvelope(id, QStringLiteral("No network adapter selected")));
                continue;
            }
            const OperationResult op = service.network()->resetDns(adapterId);
            QJsonObject result;
            result.insert(QStringLiteral("op"), opToJson(op));
            if (op.success) {
                if (service.settings()->flushCacheAfterApply())
                    service.network()->flushDnsCache();
                notify(QStringLiteral("DNS reset to DHCP"), QStringLiteral("success"));
            } else {
                notify(QStringLiteral("Could not reset DNS: %1").arg(op.message),
                       QStringLiteral("error"));
            }
            result.insert(QStringLiteral("state"), service.fullState());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("flushCache")) {
            const OperationResult op = service.network()->flushDnsCache();
            QJsonObject result;
            result.insert(QStringLiteral("op"), opToJson(op));
            notify(op.success ? QStringLiteral("DNS resolver cache flushed")
                              : QStringLiteral("Could not flush the DNS cache: %1").arg(op.message),
                   op.success ? QStringLiteral("success") : QStringLiteral("error"));
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("benchmark")) {
            QJsonObject bench = service.benchmark();
            const QString text = bench.take(QStringLiteral("noticeText")).toString();
            const QString kind = bench.take(QStringLiteral("noticeKind")).toString();
            if (!text.isEmpty())
                notify(text, kind.isEmpty() ? QStringLiteral("info") : kind);
            QJsonObject result;
            result.insert(QStringLiteral("benchmark"), bench);
            result.insert(QStringLiteral("state"), service.fullState());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.list")) {
            QJsonObject result;
            result.insert(QStringLiteral("profiles"),
                          service.profilesJson(params.value(QStringLiteral("search")).toString()));
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.add")) {
            const QString newId = service.profiles()->addProfile(CoreService::toProfileFields(params));
            if (newId.isEmpty()) {
                emitLine(errorEnvelope(
                    id, QStringLiteral("Invalid profile: a name and at least one valid DNS address are required")));
                continue;
            }
            notify(QStringLiteral("Profile saved"), QStringLiteral("success"));
            QJsonObject result;
            result.insert(QStringLiteral("id"), newId);
            result.insert(QStringLiteral("profiles"), service.profilesJson());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.update")) {
            const QString profileId = params.value(QStringLiteral("id")).toString();
            if (!service.profiles()->updateProfile(profileId, CoreService::toProfileFields(params))) {
                emitLine(errorEnvelope(id, QStringLiteral("Invalid profile data or unknown id")));
                continue;
            }
            notify(QStringLiteral("Profile updated"), QStringLiteral("success"));
            QJsonObject result;
            result.insert(QStringLiteral("profiles"), service.profilesJson());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.remove")) {
            const QString profileId = params.value(QStringLiteral("id")).toString();
            if (!service.profiles()->removeProfile(profileId)) {
                emitLine(errorEnvelope(id, QStringLiteral("Unknown profile id")));
                continue;
            }
            notify(QStringLiteral("Profile deleted"), QStringLiteral("success"));
            QJsonObject result;
            result.insert(QStringLiteral("profiles"), service.profilesJson());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.toggleFavorite")) {
            const QString profileId = params.value(QStringLiteral("id")).toString();
            if (!service.profiles()->toggleFavorite(profileId)) {
                emitLine(errorEnvelope(id, QStringLiteral("Unknown profile id")));
                continue;
            }
            QJsonObject result;
            result.insert(QStringLiteral("profiles"), service.profilesJson());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.import")) {
            const QString path = params.value(QStringLiteral("path")).toString();
            if (!CoreService::isSafeJsonPath(path) || !QFile::exists(path)) {
                emitLine(errorEnvelope(id, QStringLiteral("Invalid import file")));
                continue;
            }
            if (!service.profiles()->importFromFile(path)) {
                emitLine(errorEnvelope(
                    id, QStringLiteral("Could not import profiles: the file may be missing or invalid")));
                notify(QStringLiteral("Could not import profiles"), QStringLiteral("error"));
                continue;
            }
            notify(QStringLiteral("Profiles imported"), QStringLiteral("success"));
            QJsonObject result;
            result.insert(QStringLiteral("profiles"), service.profilesJson());
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.export")) {
            const QString path = params.value(QStringLiteral("path")).toString();
            if (!CoreService::isSafeJsonPath(path)
                || !QFileInfo(path).absoluteDir().exists()) {
                emitLine(errorEnvelope(id, QStringLiteral("Invalid export destination")));
                continue;
            }
            if (!service.profiles()->exportToFile(path)) {
                emitLine(errorEnvelope(id, QStringLiteral("Could not export profiles")));
                continue;
            }
            notify(QStringLiteral("Profiles exported"), QStringLiteral("success"));
            QJsonObject result;
            result.insert(QStringLiteral("path"), path);
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("profiles.reset")) {
            service.settings()->resetToDefaults();
            service.profiles()->resetToDefaults();
            notify(QStringLiteral("Configuration reset to defaults"), QStringLiteral("success"));
            emitLine(okEnvelope(id, service.fullState()));
        } else if (method == QStringLiteral("settings.get")) {
            QJsonObject result;
            result.insert(QStringLiteral("settings"), settingsToJson(service.settings()));
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("settings.update")) {
            AppSettings* settings = service.settings();
            if (params.contains(QStringLiteral("themeMode")))
                settings->setThemeMode(params.value(QStringLiteral("themeMode")).toString());
            const auto setBool = [&](const char* key,
                                     void (AppSettings::*setter)(bool)) {
                if (params.contains(QLatin1String(key)))
                    (settings->*setter)(params.value(QLatin1String(key)).toBool());
            };
            setBool("startWithWindows", &AppSettings::setStartWithWindows);
            setBool("startMinimized", &AppSettings::setStartMinimized);
            setBool("minimizeToTray", &AppSettings::setMinimizeToTray);
            setBool("flushCacheAfterApply", &AppSettings::setFlushCacheAfterApply);
            setBool("confirmBeforeApply", &AppSettings::setConfirmBeforeApply);
            QJsonObject result;
            result.insert(QStringLiteral("settings"), settingsToJson(settings));
            emitLine(okEnvelope(id, result));
        } else if (method == QStringLiteral("settings.reset")) {
            service.settings()->resetToDefaults();
            QJsonObject result;
            result.insert(QStringLiteral("settings"),
                          settingsToJson(service.settings()));
            emitLine(okEnvelope(id, result));
        } else {
            emitLine(errorEnvelope(id, QStringLiteral("Unknown method: %1").arg(method)));
        }
    }

    return 0;
}
