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

#include "KeyboardLayoutSearch.h"

#include "KeyboardLayoutTranslator.h"
#include "KeyboardLayouts.h"
#include "core/Config.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace
{
    // Search syntax kept as typed in every variant: the wildcards of
    // Tools::convertToRegex (*, ?, |) and the space of quoted words
    const QString PassThroughCharacters = QStringLiteral("*?| ");

    constexpr int MinimumLengthMin = 1;
    constexpr int MinimumLengthMax = 10;
} // namespace

EntrySearcher::WordVariants keyboardLayoutSearchVariants()
{
    if (!keyboardLayoutsSupported() || !config()->get(Config::Rebased_SearchAllKeyboardLayouts).toBool()) {
        return {};
    }

    const auto layouts = installedKeyboardLayouts();
    if (layouts.size() < 2) {
        return {};
    }

    const auto minimumLength = qBound(
        MinimumLengthMin, config()->get(Config::Rebased_SearchKeyboardLayoutsMinimumLength).toInt(), MinimumLengthMax);
    const KeyboardLayoutTranslator translator(layouts, minimumLength, PassThroughCharacters);
    return [translator](const QString& word) { return translator.variants(word); };
}

KeyboardLayoutSearchSettingsWidget::KeyboardLayoutSearchSettingsWidget(QWidget* parent)
    : QGroupBox(tr("Search"), parent)
    , m_enabledCheckBox(new QCheckBox(tr("Search in all keyboard layouts"), this))
    , m_minimumLengthLabel(new QLabel(tr("Minimum word length:"), this))
    , m_minimumLengthSpinBox(new QSpinBox(this))
{
    setObjectName("keyboardLayoutSearchGroup");
    m_enabledCheckBox->setObjectName("searchAllKeyboardLayoutsCheckBox");
    m_enabledCheckBox->setToolTip(
        tr("Words typed in the search bar also match as if the same keys had been pressed in any other "
           "keyboard layout of the system"));

    m_minimumLengthSpinBox->setObjectName("searchKeyboardLayoutsMinimumLengthSpinBox");
    m_minimumLengthSpinBox->setRange(MinimumLengthMin, MinimumLengthMax);
    m_minimumLengthSpinBox->setAccessibleName(tr("Minimum word length for searching in all keyboard layouts"));
    m_minimumLengthLabel->setBuddy(m_minimumLengthSpinBox);

    auto* minimumLengthLayout = new QHBoxLayout();
    minimumLengthLayout->addWidget(m_minimumLengthLabel);
    minimumLengthLayout->addWidget(m_minimumLengthSpinBox);
    minimumLengthLayout->addStretch();

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_enabledCheckBox);
    layout->addLayout(minimumLengthLayout);

    connect(m_enabledCheckBox, &QCheckBox::toggled, this, &KeyboardLayoutSearchSettingsWidget::updateEnabled);
}

void KeyboardLayoutSearchSettingsWidget::updateEnabled()
{
    m_minimumLengthLabel->setEnabled(m_enabledCheckBox->isChecked());
    m_minimumLengthSpinBox->setEnabled(m_enabledCheckBox->isChecked());
}

void KeyboardLayoutSearchSettingsWidget::loadSettings()
{
    m_enabledCheckBox->setChecked(config()->get(Config::Rebased_SearchAllKeyboardLayouts).toBool());
    m_minimumLengthSpinBox->setValue(config()->get(Config::Rebased_SearchKeyboardLayoutsMinimumLength).toInt());
    updateEnabled();
}

void KeyboardLayoutSearchSettingsWidget::saveSettings()
{
    config()->set(Config::Rebased_SearchAllKeyboardLayouts, m_enabledCheckBox->isChecked());
    config()->set(Config::Rebased_SearchKeyboardLayoutsMinimumLength, m_minimumLengthSpinBox->value());
}
