#include <QtTest>

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "core/ProfileStorage.hpp"

class TestProfileStorage : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void roundTripExplicitPath();
    void missingFileReportsError();
    void invalidJsonReportsError();
    void unsupportedVersionReportsError();
    void saveCreatesParentDirectories();
    void defaultPathMissingFileLoadsOk();
    void defaultPathCorruptFileFails();
};

static QList<DnsProfile> sampleProfiles()
{
    QList<DnsProfile> list;
    DnsProfile a = DnsProfile::create(QStringLiteral("Cloudflare"));
    a.primaryIpv4 = QStringLiteral("1.1.1.1");
    a.secondaryIpv4 = QStringLiteral("1.0.0.1");
    a.primaryIpv6 = QStringLiteral("2606:4700:4700::1111");
    a.isFavorite = true;

    DnsProfile b = DnsProfile::create(QStringLiteral("Quad9"));
    b.primaryIpv4 = QStringLiteral("9.9.9.9");
    return { a, b };
}

void TestProfileStorage::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestProfileStorage::roundTripExplicitPath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("nested/dns_profiles.json"));

    const QList<DnsProfile> original = sampleProfiles();
    const OperationResult save = ProfileStorage::saveToFile(path, original);
    QVERIFY2(save.success, qPrintable(save.message));

    QList<DnsProfile> loaded;
    const OperationResult load = ProfileStorage::loadFromFile(path, loaded);
    QVERIFY2(load.success, qPrintable(load.message));
    QCOMPARE(loaded, original);
}

void TestProfileStorage::missingFileReportsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QList<DnsProfile> loaded;
    const OperationResult result =
        ProfileStorage::loadFromFile(dir.filePath(QStringLiteral("nope.json")), loaded);
    QVERIFY(!result.success);
    QVERIFY(loaded.isEmpty());
}

void TestProfileStorage::invalidJsonReportsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("bad.json"));

    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{ this is not json !");
    file.close();

    QList<DnsProfile> loaded;
    const OperationResult result = ProfileStorage::loadFromFile(path, loaded);
    QVERIFY(!result.success);
    QVERIFY(loaded.isEmpty());
}

void TestProfileStorage::unsupportedVersionReportsError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("future.json"));

    QJsonObject root;
    root.insert(QStringLiteral("version"), ProfileStorage::kFileVersion + 99);
    root.insert(ProfileStorage::kProfileArrayKey, QJsonArray());

    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    file.close();

    QList<DnsProfile> loaded;
    const OperationResult result = ProfileStorage::loadFromFile(path, loaded);
    QVERIFY(!result.success);
    QVERIFY(loaded.isEmpty());
}

void TestProfileStorage::saveCreatesParentDirectories()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("a/b/c/dns_profiles.json"));

    const OperationResult result = ProfileStorage::saveToFile(path, sampleProfiles());
    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(QFile::exists(path));
}

void TestProfileStorage::defaultPathMissingFileLoadsOk()
{
    QFile::remove(ProfileStorage::filePath());

    QList<DnsProfile> loaded;
    loaded.append(DnsProfile::create(QStringLiteral("stale")));

    const OperationResult result = ProfileStorage::load(loaded);
    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(loaded.isEmpty()); // "nothing to load" must not keep stale data
}

void TestProfileStorage::defaultPathCorruptFileFails()
{
    QDir().mkpath(QFileInfo(ProfileStorage::filePath()).absolutePath());

    QFile file(ProfileStorage::filePath());
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QStringLiteral("not json at all").toUtf8());
    file.close();

    QList<DnsProfile> loaded;
    const OperationResult result = ProfileStorage::load(loaded);
    QVERIFY(!result.success);

    file.remove();
}

QTEST_GUILESS_MAIN(TestProfileStorage)
#include "TestProfileStorage.moc"
