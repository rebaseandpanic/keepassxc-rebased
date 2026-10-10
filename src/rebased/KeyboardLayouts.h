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
#ifndef KEEPASSXC_REBASED_KEYBOARDLAYOUTS_H
#define KEEPASSXC_REBASED_KEYBOARDLAYOUTS_H

#include <QHash>
#include <QList>
#include <QString>

// A keyboard layout enabled in the operating system: the text each physical key
// produces in it, without and with Shift.
struct KeyboardLayout
{
    // Human-readable name of the layout, for diagnostics
    QString name;
    // Text produced by each physical key, indexed by keyIndex(). The same physical
    // key has the same index in every layout returned by installedKeyboardLayouts().
    // Keys producing no text (or only a dead key) are absent.
    QHash<quint32, QString> keys;

    static quint32 keyIndex(quint32 keyCode, bool shift)
    {
        return (keyCode << 1) | (shift ? 1u : 0u);
    }
};

// Keyboard layouts enabled in the operating system. Implemented per platform
// (KeyboardLayouts_mac.mm, KeyboardLayouts_win.cpp); empty where the platform
// is not supported.
QList<KeyboardLayout> installedKeyboardLayouts();

// Whether installedKeyboardLayouts() is implemented on this platform
bool keyboardLayoutsSupported();

#endif // KEEPASSXC_REBASED_KEYBOARDLAYOUTS_H
