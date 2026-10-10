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

#include "TestGuiUnlockInProgress.h"

#include "config-keepassx-tests.h"
#include "core/Config.h"
#include "crypto/Crypto.h"
#include "gui/Application.h"
#include "gui/DatabaseOpenDialog.h"
#include "gui/DatabaseOpenWidget.h"
#include "gui/DatabaseTabWidget.h"
#include "gui/DatabaseWidget.h"
#include "gui/MainWindow.h"
#include "gui/PasswordWidget.h"
#include "util/TemporaryFile.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QTabBar>
#include <QTest>
#include <QTimer>

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    app.setApplicationName("KeePassXC");
    app.setApplicationVersion(KEEPASSXC_VERSION);
    app.setQuitOnLastWindowClosed(false);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    app.applyTheme();
    QTEST_DISABLE_KEYPAD_NAVIGATION
    TestGuiUnlockInProgress tc;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&tc, argc, argv);
}

namespace
{
    // FileKeyBinary.kdbx opens with this key file alone; its format is a legacy one,
    // so the unlock shows the legacy key file warning before opening the database
    const QString KeyFile = QStringLiteral(KEEPASSX_TEST_DATA_DIR "/FileKeyBinary.key");
} // namespace

void TestGuiUnlockInProgress::initTestCase()
{
    QVERIFY(Crypto::init());
    Config::createConfigFromFile(TemporaryFile::createTempConfigFile(), {});
    config()->set(Config::AutoSaveAfterEveryChange, false);
    config()->set(Config::AutoSaveOnExit, false);
    config()->set(Config::UpdateCheckMessageShown, true);
    config()->set(Config::Security_QuickUnlock, false);
    config()->set(Config::RememberLastKeyFiles, false);
    config()->set(Config::Messages_NoLegacyKeyFileWarning, false);

    Application::bootstrap();

    m_mainWindow.reset(new MainWindow());
    m_tabWidget = m_mainWindow->findChild<DatabaseTabWidget*>("tabWidget");
    QVERIFY(m_tabWidget);
    m_mainWindow->show();
}

void TestGuiUnlockInProgress::init()
{
    m_dir.reset(new QTemporaryDir());
    QVERIFY(m_dir->isValid());
    m_pathA = m_dir->filePath("A.kdbx");
    m_pathB = m_dir->filePath("B.kdbx");
    QVERIFY(QFile::copy(QStringLiteral(KEEPASSX_TEST_DATA_DIR "/FileKeyBinary.kdbx"), m_pathA));
    QVERIFY(QFile::copy(QStringLiteral(KEEPASSX_TEST_DATA_DIR "/FileKeyBinary.kdbx"), m_pathB));

    // Both databases open locked
    m_tabWidget->addDatabaseTab(m_pathA);
    m_dbWidgetA = m_tabWidget->currentDatabaseWidget();
    m_tabWidget->addDatabaseTab(m_pathB);
    m_dbWidgetB = m_tabWidget->currentDatabaseWidget();
    QVERIFY(m_dbWidgetA && m_dbWidgetB && m_dbWidgetA != m_dbWidgetB);
    QVERIFY(m_dbWidgetA->isLocked());
    QVERIFY(m_dbWidgetB->isLocked());
    m_tabWidget->setCurrentWidget(m_dbWidgetA);
}

void TestGuiUnlockInProgress::cleanup()
{
    while (auto* widget = QApplication::activeModalWidget()) {
        widget->close();
    }
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        m_tabWidget->databaseWidgetFromIndex(i)->database()->markAsClean();
    }
    QVERIFY(m_tabWidget->closeAllDatabaseTabs());
    QCoreApplication::processEvents();
}

void TestGuiUnlockInProgress::unlockWithWait(DatabaseOpenWidget* view, const std::function<void()>& duringWait)
{
    auto* keyFile = view->findChild<PasswordWidget*>("keyFileLineEdit");
    QVERIFY(keyFile);
    keyFile->setText(KeyFile);
    auto* buttonBox = view->findChild<QDialogButtonBox*>("buttonBox");
    QVERIFY(buttonBox);

    // Runs in the nested event loop of the warning; the unlock started before it
    bool waited = false;
    QTimer poll;
    poll.setInterval(10);
    connect(&poll, &QTimer::timeout, this, [&] {
        auto* warning = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        if (!warning) {
            return;
        }
        poll.stop();
        waited = true;
        duringWait();
        warning->accept();
    });
    poll.start();

    buttonBox->button(QDialogButtonBox::Ok)->click();
    poll.stop();
    QVERIFY(waited);
}

void TestGuiUnlockInProgress::testUnlockDialogKeepsTargetAndStaysOpen()
{
    m_tabWidget->unlockAnyDatabaseInDialog(DatabaseOpenDialog::Intent::None);
    auto* dialog = m_tabWidget->findChild<DatabaseOpenDialog*>();
    QVERIFY(dialog);
    QTRY_VERIFY(dialog->isVisible());
    auto* view = dialog->findChild<DatabaseOpenWidget*>();
    auto* tabBar = dialog->findChild<QTabBar*>();
    QVERIFY(view && tabBar);
    QCOMPARE(tabBar->count(), 2);
    QCOMPARE(view->filename(), m_pathA);
    const int tabA = tabBar->currentIndex();

    bool unlocking = false;
    int tabAfterSwitch = -1;
    QString targetAfterSwitch;
    QString targetAfterRequest;
    int tabsAfterRequest = -1;
    bool visibleAfterClose = false;
    bool tabClosed = true;
    unlockWithWait(view, [&] {
        unlocking = view->unlockingDatabase();

        // Another tab of the dialog (tab bar, Ctrl+Tab)
        tabBar->setCurrentIndex(1 - tabA);
        tabAfterSwitch = tabBar->currentIndex();
        targetAfterSwitch = view->filename();

        // Another unlock request (browser, Auto-Type, Secret Service)
        m_tabWidget->unlockDatabaseInDialog(m_dbWidgetB, DatabaseOpenDialog::Intent::None);
        targetAfterRequest = view->filename();
        tabsAfterRequest = tabBar->count();

        // Escape, the close button, cancel
        QTest::keyClick(view, Qt::Key_Escape);
        dialog->reject();
        dialog->close();
        visibleAfterClose = dialog->isVisible();

        // Closing a database tab (quit, close all)
        tabClosed = m_tabWidget->closeDatabaseTab(m_dbWidgetB);
    });

    QVERIFY(unlocking);
    QCOMPARE(tabAfterSwitch, tabA);
    QCOMPARE(targetAfterSwitch, m_pathA);
    QCOMPARE(targetAfterRequest, m_pathA);
    QCOMPARE(tabsAfterRequest, 2);
    QVERIFY(visibleAfterClose);
    QVERIFY(!tabClosed);

    // The unlock finished for the database it was started for
    QTRY_VERIFY(!dialog->isVisible());
    QVERIFY(m_dbWidgetA);
    QVERIFY(m_dbWidgetB);
    QTRY_VERIFY(!m_dbWidgetA->isLocked());
    QVERIFY(m_dbWidgetB->isLocked());
}

void TestGuiUnlockInProgress::testDeleteOnCloseDialogSurvives()
{
    // As the reload of a changed database file: a dialog deleted on close, owned by
    // a database widget, for a database outside the tabs
    QPointer<DatabaseOpenDialog> dialog = new DatabaseOpenDialog(m_dbWidgetB);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->addDatabaseTab(m_dbWidgetA);
    dialog->setActiveDatabaseTab(m_dbWidgetA);
    dialog->show();
    auto* view = dialog->findChild<DatabaseOpenWidget*>();
    QVERIFY(view);
    QCOMPARE(view->filename(), m_pathA);

    bool closed = true;
    bool ownerClosed = true;
    unlockWithWait(view, [&] {
        closed = dialog->close();
        // The owner of the dialog
        ownerClosed = m_tabWidget->closeDatabaseTab(m_dbWidgetB);
        // Deletions scheduled by a close would run here
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    });

    QVERIFY(!closed);
    QVERIFY(!ownerClosed);
    QVERIFY(m_dbWidgetB);

    // The unlock finished in the dialog, then the dialog closed and went away
    QTRY_VERIFY(dialog.isNull());
    QTRY_VERIFY(!m_dbWidgetA->isLocked());
}
