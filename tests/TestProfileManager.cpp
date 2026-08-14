#include <QtTest>

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "core/ProfileManager.hpp"
#include "core/ProfileStorage.hpp"

class TestProfileManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void seedsDefaultsOnFirstLoad();
    void addProfile();
    void addProfileRejectsInvalid();
    void updateProfile();
    void removeProfile();
    void toggleFavorite();
    void importSkipsDuplicates();
    void exportMatchesLoaded();
    void resetToDefaults();
    void persistsAcrossInstances();
};

static QVariantMap validFields(const QString& name = QStringLiteral("Custom"))
{
    return {
        { QStringLiteral("name"), name },
        { QStringLiteral("provider"), QStringLiteral("Custom") },
        { QStringLiteral("primaryIpv4"), QStringLiteral("77.77.77.77") },
        { QStringLiteral("secondaryIpv4"), QStringLiteral("88.88.88.88") },
    };
}

void TestProfileManager::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QFile::remove(ProfileStorage::filePath());
}

void TestProfileManager::cleanupTestCase()
{
    QFile::remove(ProfileStorage::filePath());
}

void TestProfileManager::seedsDefaultsOnFirstLoad()
{
    QFile::remove(ProfileStorage::filePath());

    ProfileManager manager;
    QVERIFY(manager.load());
    QCOMPARE(manager.profiles().size(), 4); // Cloudflare, Google, Quad9, AdGuard
    QVERIFY(QFile::exists(ProfileStorage::filePath()));
}

void TestProfileManager::addProfile()
{
    ProfileManager manager;
    manager.load();
    const int before = manager.profiles().size();

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    const QString id = manager.addProfile(validFields());
    QVERIFY(!id.isEmpty());
    QCOMPARE(manager.profiles().size(), before + 1);
    QCOMPARE(spy.count(), 1);

    const DnsProfile* added = manager.profileById(id);
    QVERIFY(added != nullptr);
    QCOMPARE(added->name, QStringLiteral("Custom"));
    QCOMPARE(added->primaryIpv4, QStringLiteral("77.77.77.77"));
}

void TestProfileManager::addProfileRejectsInvalid()
{
    ProfileManager manager;
    manager.load();
    const int before = manager.profiles().size();

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    QVERIFY(manager.addProfile({ { QStringLiteral("name"), QString() } }).isEmpty());
    QVERIFY(manager.addProfile(validFields(QStringLiteral("  "))).isEmpty());

    QVariantMap noServers = validFields();
    noServers.insert(QStringLiteral("primaryIpv4"), QString());
    noServers.insert(QStringLiteral("secondaryIpv4"), QString());
    QVERIFY(manager.addProfile(noServers).isEmpty());

    QVariantMap badIp = validFields();
    badIp.insert(QStringLiteral("primaryIpv4"), QStringLiteral("999.1.1.1"));
    QVERIFY(manager.addProfile(badIp).isEmpty());

    QCOMPARE(manager.profiles().size(), before);
    QCOMPARE(spy.count(), 0);
}

void TestProfileManager::updateProfile()
{
    ProfileManager manager;
    manager.load();
    const QString id = manager.addProfile(validFields());

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    QVariantMap updated = validFields(QStringLiteral("Renamed"));
    updated.insert(QStringLiteral("secondaryIpv4"), QStringLiteral("1.1.1.1"));
    QVERIFY(manager.updateProfile(id, updated));

    const DnsProfile* profile = manager.profileById(id);
    QVERIFY(profile != nullptr);
    QCOMPARE(profile->name, QStringLiteral("Renamed"));
    QCOMPARE(profile->secondaryIpv4, QStringLiteral("1.1.1.1"));
    QCOMPARE(spy.count(), 1);

    QVERIFY(!manager.updateProfile(QStringLiteral("missing-id"), updated));
    QVERIFY(!manager.updateProfile(id, validFields(QStringLiteral("   "))));
}

void TestProfileManager::removeProfile()
{
    ProfileManager manager;
    manager.load();
    const QString id = manager.addProfile(validFields());
    const int before = manager.profiles().size();

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    QVERIFY(manager.removeProfile(id));
    QCOMPARE(manager.profiles().size(), before - 1);
    QVERIFY(manager.profileById(id) == nullptr);
    QCOMPARE(spy.count(), 1);

    QVERIFY(!manager.removeProfile(id));
}

void TestProfileManager::toggleFavorite()
{
    ProfileManager manager;
    manager.load();
    const QString id = manager.addProfile(validFields());

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    QVERIFY(manager.toggleFavorite(id));
    QCOMPARE(manager.profileById(id)->isFavorite, true);
    QVERIFY(manager.toggleFavorite(id));
    QCOMPARE(manager.profileById(id)->isFavorite, false);
    QCOMPARE(spy.count(), 2);

    QVERIFY(!manager.toggleFavorite(QStringLiteral("missing-id")));
}

void TestProfileManager::importSkipsDuplicates()
{
    ProfileManager manager;
    manager.load();
    const int before = manager.profiles().size();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("import.json"));

    QList<DnsProfile> incoming = {
        DnsProfile::fromJson(manager.profiles().at(0).toJson()), // duplicate id
        [] {
            DnsProfile p = DnsProfile::create(QStringLiteral("Imported"));
            p.primaryIpv4 = QStringLiteral("5.5.5.5");
            return p;
        }(),
    };
    QVERIFY(ProfileStorage::saveToFile(path, incoming).success);

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    QVERIFY(manager.importFromFile(path));
    QCOMPARE(manager.profiles().size(), before + 1); // duplicate skipped
    QVERIFY(manager.profileById(incoming.at(0).id.toString(QUuid::WithoutBraces)) != nullptr);
    QCOMPARE(spy.count(), 1);

    QVERIFY(!manager.importFromFile(dir.filePath(QStringLiteral("missing.json"))));
}

void TestProfileManager::exportMatchesLoaded()
{
    ProfileManager manager;
    manager.load();
    manager.addProfile(validFields(QStringLiteral("Exported")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("export.json"));

    QVERIFY(manager.exportToFile(path));

    ProfileManager other;
    other.load();
    QList<DnsProfile> imported;
    QVERIFY(ProfileStorage::loadFromFile(path, imported).success);

    // The freshly loaded manager has its own ids; compare by count and names.
    QCOMPARE(imported.size(), manager.profiles().size());
    QCOMPARE(imported.size(), other.profiles().size());
    QCOMPARE(imported.at(0).name, manager.profiles().at(0).name);
}

void TestProfileManager::resetToDefaults()
{
    ProfileManager manager;
    manager.load();
    manager.addProfile(validFields(QStringLiteral("Temporary")));
    QVERIFY(manager.profiles().size() > 4);

    QSignalSpy spy(&manager, &ProfileManager::profilesChanged);

    manager.resetToDefaults();
    QCOMPARE(manager.profiles().size(), 4);
    QCOMPARE(spy.count(), 1);
    QVERIFY(manager.profileById(QStringLiteral("Temporary")) == nullptr);
}

void TestProfileManager::persistsAcrossInstances()
{
    {
        ProfileManager writer;
        writer.load();
        const QString id = writer.addProfile(validFields(QStringLiteral("Persistent")));
        QVERIFY(!id.isEmpty());
    }

    ProfileManager reader;
    QVERIFY(reader.load());
    bool found = false;
    for (const DnsProfile& profile : reader.profiles()) {
        if (profile.name == QStringLiteral("Persistent")) {
            found = true;
            break;
        }
    }
    QVERIFY(found);
}

QTEST_GUILESS_MAIN(TestProfileManager)
#include "TestProfileManager.moc"
