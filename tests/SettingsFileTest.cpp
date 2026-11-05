// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "SettingsFile.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

// The settings file is read on every start, so a missing, damaged or newer
// file must never stop the app or be overwritten by mistake.
class SettingsFileTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void startsWithDefaultsWhenThereIsNoFile();
    void roundTrips();
    void writesTheVersionNumber();
    void leavesANewerFileAlone();
    void treatsAGarbledVersionAsNewer();
    void rejectsUnreasonableWindowSizes_data();
    void rejectsUnreasonableWindowSizes();
};

namespace
{
// A settings file in a fresh folder that is deleted after the test.
class TemporarySettingsFile
{
public:
    QString path() const
    {
        return m_folder.filePath(QStringLiteral("settings.ini"));
    }

private:
    QTemporaryDir m_folder;
};
}

void SettingsFileTest::startsWithDefaultsWhenThereIsNoFile()
{
    const TemporarySettingsFile settingsFile;
    const QSettings file(settingsFile.path(), QSettings::IniFormat);
    const SettingsFile::LoadResult loaded = SettingsFile::load(file);

    QVERIFY(loaded.canSave);
    QVERIFY(!loaded.values.windowSize.isValid());
}

void SettingsFileTest::roundTrips()
{
    const TemporarySettingsFile settingsFile;
    {
        QSettings file(settingsFile.path(), QSettings::IniFormat);
        SettingsFile::save(file, {QSize(1280, 800)});
    }
    const QSettings file(settingsFile.path(), QSettings::IniFormat);
    QCOMPARE(SettingsFile::load(file).values.windowSize, QSize(1280, 800));
}

void SettingsFileTest::writesTheVersionNumber()
{
    const TemporarySettingsFile settingsFile;
    QSettings file(settingsFile.path(), QSettings::IniFormat);
    SettingsFile::save(file, {});
    QCOMPARE(file.value(QStringLiteral("version")).toInt(), SettingsFile::currentVersion);
}

void SettingsFileTest::leavesANewerFileAlone()
{
    const TemporarySettingsFile settingsFile;
    QSettings file(settingsFile.path(), QSettings::IniFormat);
    file.setValue(QStringLiteral("version"), SettingsFile::currentVersion + 1);
    file.setValue(QStringLiteral("window/size"), QSize(1280, 800));

    const SettingsFile::LoadResult loaded = SettingsFile::load(file);
    QVERIFY(!loaded.canSave);
    QVERIFY(!loaded.values.windowSize.isValid());
}

void SettingsFileTest::treatsAGarbledVersionAsNewer()
{
    const TemporarySettingsFile settingsFile;
    QSettings file(settingsFile.path(), QSettings::IniFormat);
    file.setValue(QStringLiteral("version"), QStringLiteral("not a number"));
    QVERIFY(!SettingsFile::load(file).canSave);
}

void SettingsFileTest::rejectsUnreasonableWindowSizes_data()
{
    QTest::addColumn<QSize>("size");
    QTest::addColumn<bool>("kept");

    QTest::newRow("typical") << QSize(1280, 800) << true;
    QTest::newRow("smallest") << QSize(320, 240) << true;
    QTest::newRow("too narrow") << QSize(100, 800) << false;
    QTest::newRow("too short") << QSize(1280, 10) << false;
    QTest::newRow("negative") << QSize(-1280, 800) << false;
    QTest::newRow("huge") << QSize(100000, 800) << false;
    QTest::newRow("invalid") << QSize() << false;
}

void SettingsFileTest::rejectsUnreasonableWindowSizes()
{
    QFETCH(QSize, size);
    QFETCH(bool, kept);

    QCOMPARE(SettingsFile::sanitisedWindowSize(size).isValid(), kept);
}

QTEST_GUILESS_MAIN(SettingsFileTest)

#include "SettingsFileTest.moc"
