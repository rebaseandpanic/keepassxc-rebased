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

#include "QuickUnlockAutoPromptSettings.h"

#include "core/Config.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QStyle>

QuickUnlockAutoPromptSettingsWidget::QuickUnlockAutoPromptSettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_enabledCheckBox(new QCheckBox(tr("Prompt for quick unlock automatically"), this))
{
    setObjectName("quickUnlockAutoPromptWidget");
    m_enabledCheckBox->setObjectName("quickUnlockAutoPromptCheckBox");
    m_enabledCheckBox->setToolTip(
        tr("Touch ID / Windows Hello is requested as soon as the quick unlock screen of a database appears in the "
           "main window, once each time it appears"));

    // Indented as a sub-option of quick unlock
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(style()->pixelMetric(QStyle::PM_IndicatorWidth)
                                   + style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing),
                               0,
                               0,
                               0);
    layout->addWidget(m_enabledCheckBox);
    setFocusProxy(m_enabledCheckBox);
}

void QuickUnlockAutoPromptSettingsWidget::loadSettings()
{
    m_enabledCheckBox->setChecked(config()->get(Config::Rebased_QuickUnlockAutoPrompt).toBool());
}

void QuickUnlockAutoPromptSettingsWidget::saveSettings()
{
    config()->set(Config::Rebased_QuickUnlockAutoPrompt, m_enabledCheckBox->isChecked());
}

void QuickUnlockAutoPromptSettingsWidget::setQuickUnlockEnabled(bool enabled)
{
    m_enabledCheckBox->setEnabled(enabled);
}
