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

// Windows keyboard layouts: the layouts loaded for the user, read key by key
// with ToUnicodeEx.

#include "KeyboardLayouts.h"

#include <QChar>
#include <QVector>

#include <windows.h>

namespace
{
    // Physical keys are identified by their set 1 scan code, which does not
    // depend on the layout (virtual-key codes do: VK_OEM_* and even the letter
    // keys move between layouts). Every key that can produce text sits in the
    // non-extended range; the extended (0xE0-prefixed) keys are navigation,
    // modifiers and the keypad divide / enter, none of which differ between
    // layouts.
    constexpr UINT FirstScanCode = 0x01;
    constexpr UINT LastScanCode = 0x7F;

    // ToUnicodeEx wFlags bit 2: do not change the keyboard state. Without it the
    // translation of a dead key would leave the dead key pending in the kernel
    // keyboard buffer of this (GUI) thread and alter the next character the user
    // types. Honoured since Windows 10 1607; Qt 6 requires Windows 10 1809 or
    // later, so it is always available here.
    constexpr UINT DoNotChangeKeyboardState = 1u << 2;

    // ToUnicodeEx writes one character per key, a ligature, or a surrogate pair;
    // keyboard layouts define ligatures of a few UTF-16 code units at most.
    constexpr int TranslationBufferSize = 16;

    bool isKeypadKey(UINT virtualKey)
    {
        // The numeric keypad gives the same text in every layout, and its digits
        // and operators duplicate keys of the main block, so counting it as a
        // separate physical key would only make the key of a character ambiguous.
        return virtualKey >= VK_NUMPAD0 && virtualKey <= VK_DIVIDE;
    }

    bool containsControlCharacter(const QString& text)
    {
        for (const char32_t codePoint : text.toUcs4()) {
            if (QChar::category(codePoint) == QChar::Other_Control) {
                return true;
            }
        }
        return false;
    }

    // Text produced by the key with the given scan code in the layout, without or
    // with Shift; empty when the key gives no text, a dead key or a control
    // character.
    QString keyText(HKL layout, UINT scanCode, bool shift)
    {
        const UINT virtualKey = MapVirtualKeyExW(scanCode, MAPVK_VSC_TO_VK_EX, layout);
        if (virtualKey == 0 || isKeypadKey(virtualKey)) {
            return {};
        }

        // Only Shift is down; Caps Lock, Ctrl and Alt (AltGr) are off.
        BYTE keyState[256] = {};
        if (shift) {
            keyState[VK_SHIFT] = 0x80;
            keyState[VK_LSHIFT] = 0x80;
        }

        WCHAR buffer[TranslationBufferSize] = {};
        const int length = ToUnicodeEx(
            virtualKey, scanCode, keyState, buffer, TranslationBufferSize, DoNotChangeKeyboardState, layout);
        // < 0: dead key, 0: no translation, > 0: number of UTF-16 code units
        // written (the rest of the buffer is undefined).
        if (length <= 0 || length > TranslationBufferSize) {
            return {};
        }

        const QString text = QString::fromWCharArray(buffer, length);
        if (containsControlCharacter(text)) {
            return {};
        }
        return text;
    }

    QString layoutName(HKL layout)
    {
        // The low word of an input locale identifier is the language identifier,
        // the high word identifies the physical layout.
        const auto handle = reinterpret_cast<quintptr>(layout);
        const auto languageId = static_cast<LANGID>(handle & 0xFFFF);
        const QString identifier = QStringLiteral("%1").arg(static_cast<quint32>(handle), 8, 16, QLatin1Char('0'));

        WCHAR localeName[LOCALE_NAME_MAX_LENGTH] = {};
        if (LCIDToLocaleName(MAKELCID(languageId, SORT_DEFAULT), localeName, LOCALE_NAME_MAX_LENGTH, 0) == 0) {
            return identifier;
        }

        WCHAR displayName[LOCALE_NAME_MAX_LENGTH * 4] = {};
        const int displayLength =
            GetLocaleInfoEx(localeName, LOCALE_SLOCALIZEDDISPLAYNAME, displayName, LOCALE_NAME_MAX_LENGTH * 4);
        // The returned length includes the terminating null character
        const QString name = displayLength > 1 ? QString::fromWCharArray(displayName, displayLength - 1)
                                               : QString::fromWCharArray(localeName);
        return QStringLiteral("%1 (%2)").arg(name, identifier);
    }
} // namespace

QList<KeyboardLayout> installedKeyboardLayouts()
{
    const int count = GetKeyboardLayoutList(0, nullptr);
    if (count <= 0) {
        return {};
    }
    QVector<HKL> handles(count);
    const int received = GetKeyboardLayoutList(count, handles.data());
    if (received <= 0) {
        return {};
    }
    // The list may have shrunk between the two calls
    handles.resize(qMin(received, count));

    QList<KeyboardLayout> layouts;
    layouts.reserve(handles.size());
    for (const HKL handle : handles) {
        KeyboardLayout layout;
        layout.name = layoutName(handle);
        for (UINT scanCode = FirstScanCode; scanCode <= LastScanCode; ++scanCode) {
            for (const bool shift : {false, true}) {
                const QString text = keyText(handle, scanCode, shift);
                if (!text.isEmpty()) {
                    layout.keys.insert(KeyboardLayout::keyIndex(scanCode, shift), text);
                }
            }
        }
        layouts.append(layout);
    }
    return layouts;
}

bool keyboardLayoutsSupported()
{
    return true;
}
