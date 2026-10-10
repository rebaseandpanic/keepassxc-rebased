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

#include "KeyboardLayoutTranslator.h"

#include <algorithm>

namespace
{
    QList<char32_t> codePoints(const QString& text)
    {
        const auto ucs4 = text.toUcs4();
        return {ucs4.cbegin(), ucs4.cend()};
    }
} // namespace

KeyboardLayoutTranslator::KeyboardLayoutTranslator(const QList<KeyboardLayout>& layouts,
                                                   int minimumLength,
                                                   const QString& passThrough)
    : m_minimumLength(minimumLength)
    , m_passThrough(codePoints(passThrough))
{
    m_layouts.reserve(layouts.size());
    for (const auto& layout : layouts) {
        // Visit keys by ascending index so that the first key producing a character wins
        auto indices = layout.keys.keys();
        std::sort(indices.begin(), indices.end());

        Layout table;
        for (const auto index : indices) {
            const auto text = codePoints(layout.keys.value(index));
            if (text.size() != 1) {
                continue;
            }
            table.charOf.insert(index, text.first());
            if (!table.keyOf.contains(text.first())) {
                table.keyOf.insert(text.first(), index);
            }
        }
        m_layouts.append(table);
    }
}

QStringList KeyboardLayoutTranslator::variants(const QString& word) const
{
    const auto source = codePoints(word);
    if (source.isEmpty() || source.size() < m_minimumLength) {
        return {};
    }

    QStringList result;
    for (qsizetype from = 0; from < m_layouts.size(); ++from) {
        const auto& layoutFrom = m_layouts.at(from);
        for (qsizetype to = 0; to < m_layouts.size(); ++to) {
            if (to == from) {
                continue;
            }
            const auto& layoutTo = m_layouts.at(to);

            QList<char32_t> retyped;
            retyped.reserve(source.size());
            for (const auto ch : source) {
                if (m_passThrough.contains(ch)) {
                    retyped.append(ch);
                    continue;
                }
                const auto key = layoutFrom.keyOf.constFind(ch);
                if (key == layoutFrom.keyOf.constEnd()) {
                    break;
                }
                const auto target = layoutTo.charOf.constFind(key.value());
                if (target == layoutTo.charOf.constEnd()) {
                    break;
                }
                retyped.append(target.value());
            }
            if (retyped.size() != source.size()) {
                continue;
            }

            const auto variant = QString::fromUcs4(retyped.constData(), retyped.size());
            if (variant != word && !result.contains(variant)) {
                result.append(variant);
            }
        }
    }
    return result;
}
