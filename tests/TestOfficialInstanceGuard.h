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

#ifndef KEEPASSXC_TESTOFFICIALINSTANCEGUARD_H
#define KEEPASSXC_TESTOFFICIALINSTANCEGUARD_H

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>

class TestOfficialInstanceGuard : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testOfficialPaths();
    void testAcquireWithoutOfficial();
    void testOfficialRunning();
    void testStaleLockOfCrashedOfficial();
    void testOfficialStartedLaterReachesFork();

private:
    QString lockPath() const;
    QString serverName() const;
    void startHolder(QProcess& holder, const QString& mode);

    QScopedPointer<QTemporaryDir> m_dir;
};

#endif // KEEPASSXC_TESTOFFICIALINSTANCEGUARD_H
