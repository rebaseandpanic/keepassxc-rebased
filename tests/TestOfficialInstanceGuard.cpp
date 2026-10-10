/*
 *  Copyright (C) 2026 rebaseandpanic <https://github.com/rebaseandpanic>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TestOfficialInstanceGuard.h"

#include "config-keepassx-tests.h"
#include "rebased/OfficialInstanceGuard.h"

#include <QFile>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>
#include <QTest>
#include <QUuid>

QTEST_GUILESS_MAIN(TestOfficialInstanceGuard)

void TestOfficialInstanceGuard::init()
{
    m_dir.reset(new QTemporaryDir());
    QVERIFY(m_dir->isValid());
}

void TestOfficialInstanceGuard::cleanup()
{
    m_dir.reset();
}

QString TestOfficialInstanceGuard::lockPath() const
{
    return m_dir->filePath("keepassxc-tester.lock");
}

QString TestOfficialInstanceGuard::serverName() const
{
    // Unique per test run so that parallel runs and real instances are not touched
    return QStringLiteral("kpxc-test-%1.socket").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

void TestOfficialInstanceGuard::startHolder(QProcess& holder, const QString& mode)
{
    holder.start(QStringLiteral(OFFICIAL_INSTANCE_HOLDER), {lockPath(), mode});
    QVERIFY(holder.waitForStarted());
    QVERIFY(holder.waitForReadyRead());
    QCOMPARE(holder.readLine().trimmed(), QByteArray("locked"));
}

void TestOfficialInstanceGuard::testOfficialPaths()
{
    // The official release build uses "keepassxc-<user>.lock" in the temporary directory
    // and "keepassxc-<user>.socket" as its single-instance endpoint
    const auto user = qgetenv("USER");
    const auto userName = qgetenv("USERNAME");
    qputenv("USER", "tester");
    qputenv("USERNAME", "tester");

    QCOMPARE(OfficialInstanceGuard::officialLockFilePath(),
             QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/keepassxc-tester.lock");
    QCOMPARE(OfficialInstanceGuard::officialServerName(), QStringLiteral("keepassxc-tester.socket"));

    qputenv("USER", user);
    qputenv("USERNAME", userName);
}

void TestOfficialInstanceGuard::testAcquireWithoutOfficial()
{
    {
        OfficialInstanceGuard guard(lockPath(), serverName());
        QCOMPARE(guard.acquire(), OfficialInstanceGuard::Result::Acquired);
        QVERIFY(guard.server()->isListening());
        // An official instance started now finds the lock held
        QLockFile official(lockPath());
        official.setStaleLockTime(0);
        QVERIFY(!official.tryLock(0));
        QCOMPARE(official.error(), QLockFile::LockFailedError);
    }
    // Released when the fork exits
    QVERIFY(!QFile::exists(lockPath()));
}

void TestOfficialInstanceGuard::testOfficialRunning()
{
    QProcess official;
    startHolder(official, QStringLiteral("hold"));

    OfficialInstanceGuard guard(lockPath(), serverName());
    QCOMPARE(guard.acquire(), OfficialInstanceGuard::Result::OfficialRunning);
    QVERIFY(!guard.server()->isListening());

    official.closeWriteChannel();
    QVERIFY(official.waitForFinished());
    QCOMPARE(official.exitCode(), 0);
}

void TestOfficialInstanceGuard::testStaleLockOfCrashedOfficial()
{
    QProcess official;
    startHolder(official, QStringLiteral("crash"));
    QVERIFY(official.waitForFinished());
    QVERIFY(QFile::exists(lockPath()));

    OfficialInstanceGuard guard(lockPath(), serverName());
    QCOMPARE(guard.acquire(), OfficialInstanceGuard::Result::Acquired);
}

void TestOfficialInstanceGuard::testOfficialStartedLaterReachesFork()
{
    const auto name = serverName();
    OfficialInstanceGuard guard(lockPath(), name);
    QCOMPARE(guard.acquire(), OfficialInstanceGuard::Result::Acquired);

    // The official KeePassXC, finding its lock held, connects to its endpoint to
    // hand over to the running instance and exits instead of starting
    QLocalSocket official;
    official.connectToServer(name);
    QVERIFY(official.waitForConnected(1000));
    QVERIFY(guard.server()->waitForNewConnection(1000));
}
