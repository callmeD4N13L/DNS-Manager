#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QtQml/qqml.h>

#include "models/NetworkAdapter.hpp"

// Flat QML projection of the adapters detected by NetworkManager.
class AdapterListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        NameRole,
        FriendlyNameRole,
        TypeRole,
        TypeLabelRole,
        EnabledRole,
        ConnectedRole,
        DhcpEnabledRole,
        Ipv4Role,
        Ipv6Role,
        DnsRole,
        SelectedRole
    };
    Q_ENUM(Role)

    explicit AdapterListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setAdapters(const QList<NetworkAdapter>& adapters);
    void setSelectedId(const QString& id);

    Q_INVOKABLE QVariantMap adapterAt(int row) const;

signals:
    void countChanged();

private:
    int selectedRow() const;

    QList<NetworkAdapter> m_adapters;
    QString m_selectedId;
};
