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
#ifndef KEEPASSXC_REBASED_QUICKUNLOCKAUTOPROMPT_H
#define KEEPASSXC_REBASED_QUICKUNLOCKAUTOPROMPT_H

#include <QtGlobal>

// Decides when an unlock screen prompts for Quick Unlock (Touch ID, Windows Hello)
// on its own, without the Quick Unlock button being pressed.
//
// Rule: one automatic prompt per appearance of the unlock screen. An appearance
// starts with Show and ends with Hide. The prompt waits until the window is active;
// any Quick Unlock prompt of the process during the appearance (automatic or
// manual, of this screen or of another one) uses it up. The system prompt
// deactivates and reactivates the window, so these events never cause a second
// prompt; only Hide and Show again do.
//
// Fed with the events of the unlock screen; knows nothing about widgets.
// GUI thread only.
class QuickUnlockAutoPrompt
{
public:
    // Delay between the event and the prompt, as for the unlock dialog: lets the
    // event finish and the window be painted, and skips screens only passed
    // through (fast tab switching)
    static constexpr int Delay = 100;

    // What the unlock screen knows right before the prompt
    struct State
    {
        bool enabled = false; // the option is on and the screen prompts on its own
        bool quickUnlockPossible = false; // the Quick Unlock screen is shown for this database
        bool unlocking = false; // an unlock of this screen is running
    };

    // Events of the unlock screen. Return true when a prompt may be due: the
    // screen schedules it after Delay and asks promptNow() before it starts.
    bool shown(bool windowActive);
    void hidden();
    bool windowActivated();
    void windowDeactivated();

    // Asked right before the scheduled prompt starts
    bool promptNow(const State& state) const;

    // A Quick Unlock prompt of the process, for its whole duration
    class Attempt
    {
    public:
        Attempt();
        ~Attempt();

    private:
        Q_DISABLE_COPY(Attempt)
    };

private:
    bool due() const;

    bool m_visible = false;
    bool m_windowActive = false;
    // Prompts started in the process when the screen appeared
    quint64 m_attemptsAtShow = 0;
    // A prompt was running when the screen appeared
    bool m_attemptRunningAtShow = false;
};

#endif // KEEPASSXC_REBASED_QUICKUNLOCKAUTOPROMPT_H
