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
#include "OfficialInstanceGuard.h"

#include "core/Tools.h"

#include <QStandardPaths>

namespace
{
    QString officialIdentifier()
    {
        auto identifier = QStringLiteral("keepassxc");
        auto username = Tools::cleanUsername();
        if (!username.isEmpty()) {
            identifier += QChar('-') + username;
        }
        return identifier;
    }
} // namespace

OfficialInstanceGuard::OfficialInstanceGuard(const QString& lockFilePath, const QString& serverName)
    : m_lockFile(lockFilePath)
    , m_serverName(serverName)
{
    // The lock is held for the lifetime of the application, so its age must not make
    // it stale; only a lock of a process that is no longer running is stale.
    m_lockFile.setStaleLockTime(0);
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
}

QString OfficialInstanceGuard::officialLockFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + officialIdentifier()
           + QStringLiteral(".lock");
}

QString OfficialInstanceGuard::officialServerName()
{
    return officialIdentifier() + QStringLiteral(".socket");
}

OfficialInstanceGuard::Result OfficialInstanceGuard::acquire()
{
    if (!m_lockFile.tryLock(0)) {
        if (m_lockFile.error() == QLockFile::LockFailedError) {
            return Result::OfficialRunning;
        }
        m_error = QStringLiteral("cannot create lock file %1").arg(m_lockFile.fileName());
        return Result::Unavailable;
    }

    // Holding the lock means no running instance owns the endpoint; an endpoint
    // left behind by a crashed instance would make listen() fail.
    QLocalServer::removeServer(m_serverName);
    if (!m_server.listen(m_serverName)) {
        m_error = m_server.errorString();
        m_lockFile.unlock();
        return Result::Unavailable;
    }
    return Result::Acquired;
}

QLocalServer* OfficialInstanceGuard::server()
{
    return &m_server;
}

QString OfficialInstanceGuard::errorString() const
{
    return m_error;
}
