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
#ifndef KEEPASSXC_REBASED_KEYBOARDLAYOUTSEARCH_H
#define KEEPASSXC_REBASED_KEYBOARDLAYOUTSEARCH_H

#include "core/EntrySearcher.h"

#include <QGroupBox>

class QCheckBox;
class QLabel;
class QSpinBox;

// Search in all keyboard layouts: the words of the search bar also match as if
// the same keys had been pressed in any other keyboard layout of the system.

// Variants for EntrySearcher::setWordVariants() from the application settings and
// the keyboard layouts installed right now. Empty when the option is off, the
// platform is not supported or fewer than two layouts are installed.
EntrySearcher::WordVariants keyboardLayoutSearchVariants();

// Application settings of the option: on/off and the minimum word length.
// Only shown where keyboardLayoutsSupported().
class KeyboardLayoutSearchSettingsWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit KeyboardLayoutSearchSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    void saveSettings();

private:
    void updateEnabled();

    QCheckBox* m_enabledCheckBox;
    QLabel* m_minimumLengthLabel;
    QSpinBox* m_minimumLengthSpinBox;
};

#endif // KEEPASSXC_REBASED_KEYBOARDLAYOUTSEARCH_H
