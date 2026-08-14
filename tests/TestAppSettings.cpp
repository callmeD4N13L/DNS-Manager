#include <QtTest>

#include <QSignalSpy>

#include "core/AppSettings.hpp"

class TestAppSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void defaults();
    void themeModeValidation();
    void booleanSettersEmitSignals();
    void startWithWindowsPersistence();
    void startMinimizedPersistence();
    void resetToDefaults();
    void settingsFilePathIsAbsolute();
};

void TestAppSettings::initTestCase()
{
    // Redirect QSettings and QStandardPaths to temp locations so no real
    // user config (or the HKCU Run registry entry) is touched.
    QStandardPaths::setTestModeEnabled(true);
}

void TestAppSettings::defaults()
{
    AppSettings settings;
    QCOMPARE(settings.themeMode(), QStringLiteral("system"));
    QCOMPARE(settings.startWithWindows(), false);
    QCOMPARE(settings.startMinimized(), false);
    QCOMPARE(settings.minimizeToTray(), true);
    QCOMPARE(settings.flushCacheAfterApply(), true);
    QCOMPARE(settings.confirmBeforeApply(), true);
}

void TestAppSettings::themeModeValidation()
{
    AppSettings settings;
    QSignalSpy spy(&settings, &AppSettings::themeModeChanged);

    settings.setThemeMode(QStringLiteral("DARK")); // normalized
    QCOMPARE(settings.themeMode(), QStringLiteral("dark"));
    QCOMPARE(spy.count(), 1);

    settings.setThemeMode(QStringLiteral("light"));
    QCOMPARE(settings.themeMode(), QStringLiteral("light"));
    QCOMPARE(spy.count(), 2);

    settings.setThemeMode(QStringLiteral("purple")); // rejected
    QCOMPARE(settings.themeMode(), QStringLiteral("light"));
    QCOMPARE(spy.count(), 2);

    settings.setThemeMode(QStringLiteral("light")); // no-op
    QCOMPARE(spy.count(), 2);
}

void TestAppSettings::booleanSettersEmitSignals()
{
    AppSettings settings;

    QSignalSpy traySpy(&settings, &AppSettings::minimizeToTrayChanged);
    QSignalSpy flushSpy(&settings, &AppSettings::flushCacheAfterApplyChanged);
    QSignalSpy confirmSpy(&settings, &AppSettings::confirmBeforeApplyChanged);

    settings.setMinimizeToTray(false);
    settings.setFlushCacheAfterApply(false);
    settings.setConfirmBeforeApply(false);

    QCOMPARE(settings.minimizeToTray(), false);
    QCOMPARE(settings.flushCacheAfterApply(), false);
    QCOMPARE(settings.confirmBeforeApply(), false);
    QCOMPARE(traySpy.count(), 1);
    QCOMPARE(flushSpy.count(), 1);
    QCOMPARE(confirmSpy.count(), 1);

    settings.setMinimizeToTray(false); // no-op
    QCOMPARE(traySpy.count(), 1);
}

void TestAppSettings::startWithWindowsPersistence()
{
    AppSettings writer;
    QSignalSpy spy(&writer, &AppSettings::startWithWindowsChanged);

    writer.setStartWithWindows(true);
    QCOMPARE(writer.startWithWindows(), true);
    QCOMPARE(spy.count(), 1);

    AppSettings reader;
    QCOMPARE(reader.startWithWindows(), true); // persisted

    reader.setStartWithWindows(false);
    AppSettings cleared;
    QCOMPARE(cleared.startWithWindows(), false);
}

void TestAppSettings::startMinimizedPersistence()
{
    AppSettings writer;
    writer.setStartMinimized(true);

    AppSettings reader;
    QCOMPARE(reader.startMinimized(), true);

    reader.setStartMinimized(false);
}

void TestAppSettings::resetToDefaults()
{
    AppSettings settings;
    settings.setThemeMode(QStringLiteral("dark"));
    settings.setStartWithWindows(true);
    settings.setMinimizeToTray(false);
    settings.setFlushCacheAfterApply(false);
    settings.setConfirmBeforeApply(false);

    settings.resetToDefaults();

    QCOMPARE(settings.themeMode(), QStringLiteral("system"));
    QCOMPARE(settings.startWithWindows(), false);
    QCOMPARE(settings.startMinimized(), false);
    QCOMPARE(settings.minimizeToTray(), true);
    QCOMPARE(settings.flushCacheAfterApply(), true);
    QCOMPARE(settings.confirmBeforeApply(), true);
}

void TestAppSettings::settingsFilePathIsAbsolute()
{
    AppSettings settings;
    const QString path = settings.settingsFilePath();
    QVERIFY(!path.isEmpty());
    const QFileInfo info(path);
    QVERIFY(info.isAbsolute());
    QVERIFY(info.fileName().endsWith(QStringLiteral(".ini")));
}

QTEST_GUILESS_MAIN(TestAppSettings)
#include "TestAppSettings.moc"
