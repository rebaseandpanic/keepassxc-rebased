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
#ifndef KEEPASSXC_TESTGUIUNLOCKINPROGRESS_H
#define KEEPASSXC_TESTGUIUNLOCKINPROGRESS_H

#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <functional>

#include "gui/MainWindow.h"

class DatabaseOpenWidget;
class DatabaseTabWidget;
class DatabaseWidget;

// An unlock keeps its database, its dialog and the database tabs while the event
// loop runs during it (Quick Unlock prompt, KDF, message boxes)
class TestGuiUnlockInProgress : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void testUnlockDialogKeepsTargetAndStaysOpen();
    void testDeleteOnCloseDialogSurvives();

private:
    // Starts the unlock of the target of the view with a legacy key file. The legacy
    // key file warning waits in a nested event loop; duringWait runs there, then the
    // warning is closed and the unlock continues.
    void unlockWithWait(DatabaseOpenWidget* view, const std::function<void()>& duringWait);

    QScopedPointer<MainWindow> m_mainWindow;
    QPointer<DatabaseTabWidget> m_tabWidget;
    QScopedPointer<QTemporaryDir> m_dir;
    QPointer<DatabaseWidget> m_dbWidgetA;
    QPointer<DatabaseWidget> m_dbWidgetB;
    QString m_pathA;
    QString m_pathB;
};

#endif // KEEPASSXC_TESTGUIUNLOCKINPROGRESS_H
