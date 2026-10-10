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
#ifndef KEEPASSXC_REBASED_OFFICIALINSTANCEGUARD_H
#define KEEPASSXC_REBASED_OFFICIALINSTANCEGUARD_H

#include <QLocalServer>
#include <QLockFile>
#include <QString>

// Keeps the official KeePassXC from running at the same time as the fork.
// Both share the browser proxy socket and other org.keepassxc.KeePassXC
// identifiers, so only one of them may run.
//
// The guard takes the single-instance lock of the official KeePassXC and
// listens on its single-instance endpoint for its whole lifetime. An official
// KeePassXC started later finds the lock held, reaches this endpoint, treats the
// fork as its running instance and exits.
class OfficialInstanceGuard
{
public:
    enum class Result
    {
        // The guard holds the official lock and endpoint
        Acquired,
        // A running process holds the official lock
        OfficialRunning,
        // The lock or the endpoint could not be set up; nothing is held
        Unavailable
    };

    OfficialInstanceGuard(const QString& lockFilePath, const QString& serverName);

    // Lock file and endpoint name of the official release build
    // ("keepassxc[-<user>].lock" / ".socket" in the temporary directory, see Application::Application)
    static QString officialLockFilePath();
    static QString officialServerName();

    Result acquire();

    // Server receiving the connections of official instances started later;
    // listening only after acquire() returned Acquired
    QLocalServer* server();

    QString errorString() const;

private:
    QLockFile m_lockFile;
    QLocalServer m_server;
    QString m_serverName;
    QString m_error;
};

#endif // KEEPASSXC_REBASED_OFFICIALINSTANCEGUARD_H
