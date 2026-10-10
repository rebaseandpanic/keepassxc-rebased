/*
 *  Copyright (C) 2026 KeePassXC Team <team@keepassxc.org>
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

#include "TestUpdateCheck.h"
#include "crypto/Crypto.h"
#include "networking/UpdateChecker.h"

#include <QTest>

QTEST_GUILESS_MAIN(TestUpdateCheck)

void TestUpdateCheck::initTestCase()
{
    QVERIFY(Crypto::init());
    QLocale::setDefault(QLocale::c());
}

void TestUpdateCheck::testCompareVersion()
{
    // No upgrade
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0"), QString("2.3.0")), false);

    // First digit upgrade
    QCOMPARE(UpdateChecker::compareVersions(QString("2.4.0"), QString("3.0.0")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("3.0.0"), QString("2.4.0")), false);

    // Second digit upgrade
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.4"), QString("2.4.0")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.4.0"), QString("2.3.4")), false);

    // Third digit upgrade
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0"), QString("2.3.1")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.1"), QString("2.3.0")), false);

    // Beta builds
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0"), QString("2.3.0-beta1")), false);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0"), QString("2.3.1-beta1")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0-beta1"), QString("2.3.0")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0-beta"), QString("2.3.0-beta1")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0-beta1"), QString("2.3.0-beta")), false);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0-beta1"), QString("2.3.0-beta2")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.0-beta2"), QString("2.3.0-beta1")), false);

    // Snapshot and invalid data
    QCOMPARE(UpdateChecker::compareVersions(QString("2.3.4-snapshot"), QString("2.4.0")), false);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.4.0"), QString("invalid")), false);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.4.0"), QString("")), false);
}

void TestUpdateCheck::testCompareRebasedVersion()
{
    // Next fork release on the same upstream base
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-rebased1"), QString("2.8.0-rebased2")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-rebased2"), QString("2.8.0-rebased1")), false);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-rebased1"), QString("2.8.0-rebased1")), false);
    // Release numbers are compared as numbers, not as text
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-rebased9"), QString("2.8.0-rebased10")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-rebased10"), QString("2.8.0-rebased9")), false);

    // New upstream base
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-rebased3"), QString("2.9.0-rebased1")), true);
    QCOMPARE(UpdateChecker::compareVersions(QString("2.9.0-rebased1"), QString("2.8.0-rebased3")), false);

    // Own version numbers without a suffix
    QCOMPARE(UpdateChecker::compareVersions(QString("2.9.0-rebased1"), QString("3.0.0")), true);

    // Snapshot builds of the fork are not checked
    QCOMPARE(UpdateChecker::compareVersions(QString("2.8.0-snapshot"), QString("2.8.0-rebased1")), false);
}
