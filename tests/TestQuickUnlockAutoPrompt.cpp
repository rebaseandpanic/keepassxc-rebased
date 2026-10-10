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

#include "TestQuickUnlockAutoPrompt.h"

#include "rebased/QuickUnlockAutoPrompt.h"

#include <QTest>

QTEST_GUILESS_MAIN(TestQuickUnlockAutoPrompt)

namespace
{
    // Option on, Quick Unlock screen shown, no unlock running
    QuickUnlockAutoPrompt::State ready()
    {
        QuickUnlockAutoPrompt::State state;
        state.enabled = true;
        state.quickUnlockPossible = true;
        state.unlocking = false;
        return state;
    }
} // namespace

void TestQuickUnlockAutoPrompt::testShownInActiveWindow()
{
    QuickUnlockAutoPrompt autoPrompt;
    QVERIFY(autoPrompt.shown(true));
    QVERIFY(autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testShownInInactiveWindow()
{
    QuickUnlockAutoPrompt autoPrompt;
    QVERIFY(!autoPrompt.shown(false));
    QVERIFY(!autoPrompt.promptNow(ready()));

    QVERIFY(autoPrompt.windowActivated());
    QVERIFY(autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testNoSecondPromptOnReactivation()
{
    QuickUnlockAutoPrompt autoPrompt;
    autoPrompt.shown(true);
    QVERIFY(autoPrompt.promptNow(ready()));
    {
        // The system prompt deactivates and reactivates the window while it is shown
        QuickUnlockAutoPrompt::Attempt prompt;
        autoPrompt.windowDeactivated();
        QVERIFY(!autoPrompt.windowActivated());
        QVERIFY(!autoPrompt.promptNow(ready()));
    }
    // Canceled or failed: the screen stays, the window comes back again
    QVERIFY(!autoPrompt.promptNow(ready()));
    autoPrompt.windowDeactivated();
    QVERIFY(!autoPrompt.windowActivated());
    QVERIFY(!autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testPromptAgainAfterHideAndShow()
{
    QuickUnlockAutoPrompt autoPrompt;
    autoPrompt.shown(true);
    {
        QuickUnlockAutoPrompt::Attempt prompt;
    }
    QVERIFY(!autoPrompt.promptNow(ready()));

    autoPrompt.hidden();
    QVERIFY(autoPrompt.shown(true));
    QVERIFY(autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testHiddenBeforePrompt()
{
    // Passed through: the scheduled prompt finds the screen hidden
    QuickUnlockAutoPrompt autoPrompt;
    QVERIFY(autoPrompt.shown(true));
    autoPrompt.hidden();
    QVERIFY(!autoPrompt.promptNow(ready()));
    QVERIFY(!autoPrompt.windowActivated());
}

void TestQuickUnlockAutoPrompt::testDeactivatedBeforePrompt()
{
    QuickUnlockAutoPrompt autoPrompt;
    QVERIFY(autoPrompt.shown(true));
    autoPrompt.windowDeactivated();
    QVERIFY(!autoPrompt.promptNow(ready()));

    QVERIFY(autoPrompt.windowActivated());
    QVERIFY(autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testManualPromptUsesUpAppearance()
{
    // Quick Unlock button pressed while the window was inactive
    QuickUnlockAutoPrompt autoPrompt;
    autoPrompt.shown(false);
    {
        QuickUnlockAutoPrompt::Attempt prompt;
    }
    QVERIFY(!autoPrompt.windowActivated());
    QVERIFY(!autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testOtherScreenPromptUsesUpAppearance()
{
    // The unlock dialog prompts over the main window, then closes and activates it
    QuickUnlockAutoPrompt mainWindowScreen;
    mainWindowScreen.shown(false);
    {
        QuickUnlockAutoPrompt::Attempt dialogPrompt;
        QVERIFY(!mainWindowScreen.promptNow(ready()));
    }
    QVERIFY(!mainWindowScreen.windowActivated());
    QVERIFY(!mainWindowScreen.promptNow(ready()));

    // Another screen is not used up by a prompt that ended before it appeared
    QuickUnlockAutoPrompt laterScreen;
    QVERIFY(laterScreen.shown(true));
    QVERIFY(laterScreen.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testShownWhilePromptRuns()
{
    // The screen was switched away and back while its prompt was shown
    QuickUnlockAutoPrompt autoPrompt;
    autoPrompt.shown(true);
    {
        QuickUnlockAutoPrompt::Attempt prompt;
        autoPrompt.hidden();
        QVERIFY(!autoPrompt.shown(true));
    }
    QVERIFY(!autoPrompt.windowActivated());
    QVERIFY(!autoPrompt.promptNow(ready()));
}

void TestQuickUnlockAutoPrompt::testState_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("quickUnlockPossible");
    QTest::addColumn<bool>("unlocking");
    QTest::addColumn<bool>("prompt");

    QTest::newRow("ready") << true << true << false << true;
    QTest::newRow("option off") << false << true << false << false;
    QTest::newRow("quick unlock not possible") << true << false << false << false;
    QTest::newRow("unlock in progress") << true << true << true << false;
}

void TestQuickUnlockAutoPrompt::testState()
{
    QFETCH(bool, enabled);
    QFETCH(bool, quickUnlockPossible);
    QFETCH(bool, unlocking);
    QFETCH(bool, prompt);

    QuickUnlockAutoPrompt::State state;
    state.enabled = enabled;
    state.quickUnlockPossible = quickUnlockPossible;
    state.unlocking = unlocking;

    // Shown in the active window and activated again later: the state decides each time
    QuickUnlockAutoPrompt autoPrompt;
    QVERIFY(autoPrompt.shown(true));
    QCOMPARE(autoPrompt.promptNow(state), prompt);
    autoPrompt.windowDeactivated();
    QVERIFY(autoPrompt.windowActivated());
    QCOMPARE(autoPrompt.promptNow(state), prompt);
}
