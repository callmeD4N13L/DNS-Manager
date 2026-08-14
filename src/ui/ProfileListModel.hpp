#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QtQml/qqml.h>

#include "models/DnsProfile.hpp"

class ProfileManager;

// Filterable, QML-facing view over the profiles owned by a ProfileManager.
// Mutations are delegated to the manager, which persists them and emits
// profilesChanged(); this model rebuilds its filtered view in response.
class ProfileListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ProviderRole,
        PrimaryIpv4Role,
        SecondaryIpv4Role,
        PrimaryIpv6Role,
        SecondaryIpv6Role,
        DescriptionRole,
        FavoriteRole
    };
    Q_ENUM(Role)

    explicit ProfileListModel(ProfileManager* profiles, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // QML-facing API -------------------------------------------------------
    Q_INVOKABLE QString addProfile(const QVariantMap& fields);
    Q_INVOKABLE bool updateProfile(const QString& id, const QVariantMap& fields);
    Q_INVOKABLE bool removeProfile(const QString& id);
    Q_INVOKABLE bool toggleFavorite(const QString& id);
    Q_INVOKABLE void setSearch(const QString& text);
    Q_INVOKABLE QVariantList profilesList() const;

    // C++ API --------------------------------------------------------------
    const QList<DnsProfile>& profiles() const;

signals:
    void countChanged();

private:
    void rebuild();

    ProfileManager* m_profiles = nullptr;
    QList<int> m_filtered; // rows visible after applying m_search
    QString m_search;
};
