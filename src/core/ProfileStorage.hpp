#pragma once

#include <QList>
#include <QString>

#include "core/OperationResult.hpp"
#include "models/DnsProfile.hpp"

// Persists the profile list as a small JSON document in the user's app-data
// directory (QStandardPaths::AppDataLocation). Loads never mutate the caller's
// list on failure; saves are atomic (QSaveFile).
namespace ProfileStorage {

constexpr int kFileVersion = 1;
inline const QString kProfileArrayKey = QStringLiteral("dns_profiles");

QString filePath();

// Default-location helpers: load() treats a missing file as "nothing loaded
// yet" (ok), whereas loadFromFile() reports it as an error.
OperationResult load(QList<DnsProfile>& profiles);
OperationResult save(const QList<DnsProfile>& profiles);

OperationResult loadFromFile(const QString& path, QList<DnsProfile>& profiles);
OperationResult saveToFile(const QString& path, const QList<DnsProfile>& profiles);

} // namespace ProfileStorage
