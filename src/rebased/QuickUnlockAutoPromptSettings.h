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
#ifndef KEEPASSXC_REBASED_QUICKUNLOCKAUTOPROMPTSETTINGS_H
#define KEEPASSXC_REBASED_QUICKUNLOCKAUTOPROMPTSETTINGS_H

#include <QWidget>

class QCheckBox;

// Application settings of the automatic Quick Unlock prompt: a checkbox shown
// under the Quick Unlock option, available while Quick Unlock is.
class QuickUnlockAutoPromptSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuickUnlockAutoPromptSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    void saveSettings();

    // Follows the Quick Unlock checkbox: available and checked
    void setQuickUnlockEnabled(bool enabled);

private:
    QCheckBox* m_enabledCheckBox;
};

#endif // KEEPASSXC_REBASED_QUICKUNLOCKAUTOPROMPTSETTINGS_H
