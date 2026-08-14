#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

#include "models/DnsProfile.hpp"

// Owns the list of saved DNS profiles and keeps it in sync with disk via
// ProfileStorage. Mutations validate input, persist immediately and emit
// profilesChanged(). The first run is seeded with well-known public
// resolvers so the UI is not empty.
class ProfileManager : public QObject
{
    Q_OBJECT

public:
    explicit ProfileManager(QObject* parent = nullptr);

    // Loads profiles from disk. Seeds the default set when no file exists.
    // Returns true when the in-memory list is usable afterwards.
    bool load();

    const QList<DnsProfile>& profiles() const;
    const DnsProfile* profileById(const QString& id) const;
    int indexOf(const QString& id) const;

    // Returns the new profile id, or an empty string when validation fails.
    QString addProfile(const QVariantMap& fields);
    bool updateProfile(const QString& id, const QVariantMap& fields);
    bool removeProfile(const QString& id);
    bool toggleFavorite(const QString& id);

    // Merges profiles from a JSON file (skips ids that already exist).
    bool importFromFile(const QString& path);
    bool exportToFile(const QString& path);
    void resetToDefaults();

signals:
    void profilesChanged();
    void storageError(const QString& message);

private:
    void seedDefaults();
    bool persist() const;
    static bool mapToProfile(const QVariantMap& fields, DnsProfile& out);

    QList<DnsProfile> m_profiles;
};
