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

#include "QuickUnlockAutoPrompt.h"

namespace
{
    // Quick Unlock prompts of the process: started so far and running now.
    // The system prompt is one for the whole process, so is this state.
    quint64 attemptsStarted = 0;
    int attemptsRunning = 0;
} // namespace

bool QuickUnlockAutoPrompt::shown(bool windowActive)
{
    m_visible = true;
    m_windowActive = windowActive;
    m_attemptsAtShow = attemptsStarted;
    // A prompt that runs while the screen appears (the screen was switched away
    // and back during it) belongs to this appearance
    m_attemptRunningAtShow = attemptsRunning > 0;
    return due();
}

void QuickUnlockAutoPrompt::hidden()
{
    m_visible = false;
}

bool QuickUnlockAutoPrompt::windowActivated()
{
    m_windowActive = true;
    return due();
}

void QuickUnlockAutoPrompt::windowDeactivated()
{
    m_windowActive = false;
}

bool QuickUnlockAutoPrompt::promptNow(const State& state) const
{
    return state.enabled && state.quickUnlockPossible && !state.unlocking && due();
}

bool QuickUnlockAutoPrompt::due() const
{
    return m_visible && m_windowActive && !m_attemptRunningAtShow && attemptsStarted == m_attemptsAtShow
           && attemptsRunning == 0;
}

QuickUnlockAutoPrompt::Attempt::Attempt()
{
    ++attemptsStarted;
    ++attemptsRunning;
}

QuickUnlockAutoPrompt::Attempt::~Attempt()
{
    --attemptsRunning;
}
