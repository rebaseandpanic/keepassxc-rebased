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
#ifndef KEEPASSXC_REBASED_UNLOCKSTATE_H
#define KEEPASSXC_REBASED_UNLOCKSTATE_H

#include <QtGlobal>

// Whether an unlock screen (DatabaseOpenWidget) has an unlock running: from the
// start of DatabaseOpenWidget::openDatabase() until its result is known.
//
// The event loop keeps running during an unlock (the Quick Unlock prompt, the
// KDF, message boxes), so events can arrive that change the database of the
// screen, close its dialog or close database tabs. While an unlock runs, the
// screen keeps its database, its dialog stays open and no database tab closes:
// they ask running() and anyRunning() and refuse.
//
// GUI thread only.
class UnlockState
{
public:
    UnlockState() = default;
    ~UnlockState();

    bool running() const;
    void setRunning(bool running);

    // An unlock runs in any unlock screen of the process
    static bool anyRunning();

private:
    Q_DISABLE_COPY(UnlockState)

    bool m_running = false;
};

#endif // KEEPASSXC_REBASED_UNLOCKSTATE_H
