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

#include "UnlockState.h"

namespace
{
    // Unlock screens of the process with an unlock running
    int unlocksRunning = 0;
} // namespace

UnlockState::~UnlockState()
{
    setRunning(false);
}

bool UnlockState::running() const
{
    return m_running;
}

void UnlockState::setRunning(bool running)
{
    if (running == m_running) {
        return;
    }
    m_running = running;
    unlocksRunning += running ? 1 : -1;
}

bool UnlockState::anyRunning()
{
    return unlocksRunning > 0;
}
