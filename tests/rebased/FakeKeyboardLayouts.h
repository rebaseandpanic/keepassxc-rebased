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
#ifndef KEEPASSXC_TEST_FAKEKEYBOARDLAYOUTS_H
#define KEEPASSXC_TEST_FAKEKEYBOARDLAYOUTS_H

#include "rebased/KeyboardLayouts.h"

// Keyboard layout tables for tests, written down from the physical keyboard.
// Character i of plain and shifted is produced by physical key i without and with
// Shift; a space marks a key that produces nothing.
inline KeyboardLayout fakeKeyboardLayout(const QString& name, const QString& plain, const QString& shifted)
{
    KeyboardLayout layout;
    layout.name = name;
    const auto addLevel = [&layout](const QString& level, bool shift) {
        const auto keys = level.toUcs4();
        for (int key = 0; key < keys.size(); ++key) {
            if (keys.at(key) != ' ') {
                const char32_t ch = keys.at(key);
                layout.keys.insert(KeyboardLayout::keyIndex(key, shift), QString::fromUcs4(&ch, 1));
            }
        }
    };
    addLevel(plain, false);
    addLevel(shifted, true);
    return layout;
}

// US English (QWERTY): digit row, letter rows and the punctuation keys among them, backslash after "]"
inline KeyboardLayout fakeEnglishLayout()
{
    return fakeKeyboardLayout(QStringLiteral("English"),
                              QStringLiteral("`1234567890qwertyuiop[]\\asdfghjkl;'zxcvbnm,./"),
                              QStringLiteral("~!@#$%^&*()QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?"));
}

// Russian (JCUKEN) on the same physical keys
inline KeyboardLayout fakeRussianLayout()
{
    return fakeKeyboardLayout(QStringLiteral("Russian"),
                              QStringLiteral("ё1234567890йцукенгшщзхъ\\фывапролджэячсмитьбю."),
                              QStringLiteral("Ё!\"№;%:?*()ЙЦУКЕНГШЩЗХЪ/ФЫВАПРОЛДЖЭЯЧСМИТЬБЮ,"));
}

#endif // KEEPASSXC_TEST_FAKEKEYBOARDLAYOUTS_H
