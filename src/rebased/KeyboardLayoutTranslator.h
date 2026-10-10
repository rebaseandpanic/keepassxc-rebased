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
#ifndef KEEPASSXC_REBASED_KEYBOARDLAYOUTTRANSLATOR_H
#define KEEPASSXC_REBASED_KEYBOARDLAYOUTTRANSLATOR_H

#include "KeyboardLayouts.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

// Retypes a word in other keyboard layouts: the text that appears when the keys
// typing the word in one layout are pressed while another layout is active.
// With English and Russian layouts "ецшееук" becomes "twitter" and back.
//
// Only single code points take part in the mapping. A key whose text is empty or
// longer than one code point (dead keys, ligatures) is ignored. When several keys
// of a layout produce the same character, the key with the lowest index
// (KeyboardLayout::keyIndex) is used, so a plain key wins over a shifted one.
class KeyboardLayoutTranslator
{
public:
    // layouts:       layouts to translate between, key indices shared by all of them
    // minimumLength: words shorter than this many code points are not translated
    // passThrough:   characters copied unchanged into every variant, whatever the
    //                layouts produce for them (for example search wildcards)
    KeyboardLayoutTranslator(const QList<KeyboardLayout>& layouts, int minimumLength, const QString& passThrough = {});

    // Variants of the word: for each ordered pair of different layouts (A, B) whose
    // layout A produces every character of the word, the word retyped from A into B.
    // A pair is skipped when B produces no single character for one of the keys.
    // Variants equal to the word and repeated variants are left out; the order
    // follows the order of the layouts.
    QStringList variants(const QString& word) const;

private:
    struct Layout
    {
        // Character -> index of the key producing it
        QHash<char32_t, quint32> keyOf;
        // Index of the key -> character it produces
        QHash<quint32, char32_t> charOf;
    };

    QList<Layout> m_layouts;
    int m_minimumLength;
    QList<char32_t> m_passThrough;
};

#endif // KEEPASSXC_REBASED_KEYBOARDLAYOUTTRANSLATOR_H
