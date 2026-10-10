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
// Stands in for a running or crashed official KeePassXC in TestOfficialInstanceGuard:
// takes the lock file given as the first argument and prints "locked".
//   hold  - keeps the lock until stdin is closed, then exits normally
//   crash - exits immediately without releasing the lock, leaving a stale lock file

#include <QCoreApplication>
#include <QLockFile>
#include <QTextStream>

#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    const auto args = app.arguments();
    if (args.size() != 3) {
        return 2;
    }

    QLockFile lockFile(args.at(1));
    lockFile.setStaleLockTime(0);
    if (!lockFile.tryLock(0)) {
        return 1;
    }
    std::fputs("locked\n", stdout);
    std::fflush(stdout);

    if (args.at(2) == QLatin1String("crash")) {
        std::_Exit(0);
    }

    QTextStream in(stdin);
    while (!in.atEnd()) {
        in.readLine();
    }
    return 0;
}
