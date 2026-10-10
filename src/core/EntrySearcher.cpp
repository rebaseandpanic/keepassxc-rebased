/*
 *  Copyright (C) 2014 Florian Geyer <blueice@fobos.de>
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
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

#include "EntrySearcher.h"

#include "PasswordHealth.h"
#include "core/Group.h"
#include "core/Tools.h"
#include "rebased/KeyboardLayoutTranslator.h"

#include <algorithm>

EntrySearcher::EntrySearcher(bool caseSensitive, bool skipProtected)
    : m_caseSensitive(caseSensitive)
    , m_skipProtected(skipProtected)
{
}

/**
 * Search group, and its children, directly by provided search terms
 * @param searchTerms  search terms
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::search(const QList<SearchTerm>& searchTerms, const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);
    m_searchTerms = searchTerms;
    return repeat(baseGroup, forceSearch);
}

/**
 * Search group, and its children, by parsing the provided search
 * string for search terms.
 *
 * @param searchString search terms
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::search(const QString& searchString, const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);
    parseSearchTerms(searchString);
    return repeat(baseGroup, forceSearch);
}

/**
 * Repeat the last search starting from the given group
 *
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::repeat(const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);

    QList<Entry*> results;
    for (const auto group : baseGroup->groupsRecursive(true)) {
        if (forceSearch || group->resolveSearchingEnabled()) {
            for (const auto entry : group->entries()) {
                if (searchEntryImpl(entry)) {
                    results.append(entry);
                }
            }
        }
    }
    return results;
}

/**
 * Search provided entries by the provided search terms
 *
 * @param searchTerms search terms
 * @param entries list of entries to include in the search
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::searchEntries(const QList<SearchTerm>& searchTerms, const QList<Entry*>& entries)
{
    m_searchTerms = searchTerms;
    return repeatEntries(entries);
}

/**
 * Search provided entries by parsing the search string
 * for search terms.
 *
 * @param searchString search terms
 * @param entries list of entries to include in the search
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::searchEntries(const QString& searchString, const QList<Entry*>& entries)
{
    parseSearchTerms(searchString);
    return repeatEntries(entries);
}

/**
 * Repeat the last search on the given entries
 *
 * @param entries list of entries to include in the search
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::repeatEntries(const QList<Entry*>& entries)
{
    QList<Entry*> results;
    for (auto* entry : entries) {
        if (searchEntryImpl(entry)) {
            results.append(entry);
        }
    }
    return results;
}

/**
 * Set the next search to be case sensitive or not
 *
 * @param state
 */
void EntrySearcher::setCaseSensitive(bool state)
{
    m_caseSensitive = state;
}

bool EntrySearcher::isCaseSensitive() const
{
    return m_caseSensitive;
}

/**
 * Set the source of alternative spellings for the words of the next parsed search strings
 *
 * @param wordVariants returns the variants of a word, empty to search the words only as typed
 */
void EntrySearcher::setWordVariants(const WordVariants& wordVariants)
{
    m_wordVariants = wordVariants;
}

bool EntrySearcher::searchEntryImpl(const Entry* entry)
{
    // Pre-load in case they are needed
    auto attributes_keys = entry->attributes()->customKeys();
    auto attributes = QStringList(attributes_keys + entry->attributes()->values(attributes_keys));
    auto attachments = QStringList(entry->attachments()->keys());
    // Build a group hierarchy to allow searching for e.g. /group1/subgroup*
    QString hierarchy;
    if (entry->group()) {
        hierarchy = entry->group()->hierarchy().join('/').prepend("/");
    }

    // By default, empty term matches every entry.
    // However when skipping protected fields, we will reject everything instead
    bool found = !m_skipProtected;
    for (const auto& term : m_searchTerms) {
        // The term matches if its word or any of its alternatives matches
        const auto matchAny = [&term](const auto& matches) {
            return matches(term.word, term.regex)
                   || std::any_of(term.alternatives.cbegin(), term.alternatives.cend(), [&matches](const auto& alt) {
                          return matches(alt.word, alt.regex);
                      });
        };
        const auto matchText = [&matchAny](const QString& text) {
            return matchAny(
                [&text](const QString&, const QRegularExpression& regex) { return regex.match(text).hasMatch(); });
        };

        switch (term.field) {
        case Field::Title:
            found = matchText(entry->resolvePlaceholder(entry->title()));
            break;
        case Field::Username:
            found = matchText(entry->resolvePlaceholder(entry->username()));
            break;
        case Field::Password:
            if (m_skipProtected) {
                continue;
            }
            found = term.regex.match(entry->resolvePlaceholder(entry->password())).hasMatch();
            break;
        case Field::Url:
            found = matchText(entry->resolvePlaceholder(entry->url()));
            break;
        case Field::Notes:
            found = matchText(entry->notes());
            break;
        case Field::AttributeKV:
            found = matchAny([&attributes](const QString&, const QRegularExpression& regex) {
                return !attributes.filter(regex).empty();
            });
            break;
        case Field::Attachment:
            found = matchAny([&attachments](const QString&, const QRegularExpression& regex) {
                return !attachments.filter(regex).empty();
            });
            break;
        case Field::AttributeValue:
            if (m_skipProtected && entry->attributes()->isProtected(term.word)) {
                continue;
            }
            found = entry->attributes()->contains(term.word) && matchText(entry->attributes()->value(term.word));
            break;
        case Field::Group:
            // Match against the full hierarchy if the word contains a '/' otherwise just the group name
            // (an entry without a group keeps the previous result)
            found =
                matchAny([&entry, &hierarchy, previous = found](const QString& word, const QRegularExpression& regex) {
                    if (word.contains('/')) {
                        return regex.match(hierarchy).hasMatch();
                    } else if (entry->group()) {
                        return regex.match(entry->group()->name()).hasMatch();
                    }
                    return previous;
                });
            break;
        case Field::Tag:
            found = matchAny([&entry](const QString&, const QRegularExpression& regex) {
                return entry->tagList().indexOf(regex) != -1;
            });
            break;
        case Field::Is:
            if (term.word.startsWith("expired", Qt::CaseInsensitive)) {
                auto days = 0;
                auto parts = term.word.split("-", Qt::SkipEmptyParts);
                if (parts.length() >= 2) {
                    days = parts[1].toInt();
                }
                found = entry->willExpireInDays(days) && !entry->isRecycled();
                break;
            } else if (term.word.compare("weak", Qt::CaseInsensitive) == 0) {
                if (!entry->excludeFromReports() && !entry->password().isEmpty() && !entry->isExpired()) {
                    const auto quality = entry->passwordHealth()->quality();
                    if (quality == PasswordHealth::Quality::Bad || quality == PasswordHealth::Quality::Poor
                        || quality == PasswordHealth::Quality::Weak) {
                        found = true;
                        break;
                    }
                }
            }
            found = false;
            break;
        case Field::Has:
            if (term.word.compare("totp", Qt::CaseInsensitive) == 0) {
                found = entry->hasTotp();
                break;
            }
            found = false;
            break;
        case Field::Uuid:
            found = term.regex.match(entry->uuidToHex()).hasMatch();
            break;
        default:
            // Terms without a specific field try to match title, username, url, and notes
            found = matchAny([&entry](const QString&, const QRegularExpression& regex) {
                return regex.match(entry->resolvePlaceholder(entry->title())).hasMatch()
                       || regex.match(entry->resolvePlaceholder(entry->username())).hasMatch()
                       || regex.match(entry->resolvePlaceholder(entry->url())).hasMatch()
                       || entry->tagList().indexOf(regex) != -1 || regex.match(entry->notes()).hasMatch();
            });
        }

        // negate the result if exclude:
        // * if found and not excluding, the entry matches
        // * if didn't found but excluding, the entry also matches
        found = (found && !term.exclude) || (!found && term.exclude);

        // short circuit if we failed the match
        if (!found) {
            return false;
        }
    }

    return found;
}

void EntrySearcher::parseSearchTerms(const QString& searchString)
{
    static const QList<QPair<QString, Field>> fieldnames{
        {QStringLiteral("attachment"), Field::Attachment},
        {QStringLiteral("attribute"), Field::AttributeKV},
        {QStringLiteral("notes"), Field::Notes},
        {QStringLiteral("pw"), Field::Password},
        {QStringLiteral("password"), Field::Password},
        {QStringLiteral("title"), Field::Title}, // title before tag to capture t:<word>
        {QStringLiteral("username"), Field::Username}, // username before url to capture u:<word>
        {QStringLiteral("url"), Field::Url},
        {QStringLiteral("group"), Field::Group},
        {QStringLiteral("tag"), Field::Tag},
        {QStringLiteral("is"), Field::Is},
        {QStringLiteral("has"), Field::Has},
        {QStringLiteral("uuid"), Field::Uuid}};

    // Group 1 = modifiers, Group 2 = field, Group 3 = quoted string, Group 4 = unquoted string
    static QRegularExpression termParser(R"re(([-!*+]+)?(?:(\w*):)?(?:(?=")"((?:[^"\\]|\\.)*)"|([^ ]*))( |$))re");

    m_searchTerms.clear();
    auto results = termParser.globalMatch(searchString);
    while (results.hasNext()) {
        auto result = results.next();
        SearchTerm term{};

        // Quoted string group
        term.word = result.captured(3);
        // Unescape quotes
        term.word.replace("\\\"", "\"");

        // If empty, use the unquoted string group
        if (term.word.isEmpty()) {
            term.word = result.captured(4);
        }

        // If still empty, ignore this match
        if (term.word.isEmpty()) {
            continue;
        }

        auto mods = result.captured(1);

        // Convert term to regex
        int opts = m_caseSensitive ? Tools::RegexConvertOpts::CASE_SENSITIVE : Tools::RegexConvertOpts::DEFAULT;
        if (!mods.contains("*")) {
            opts |= Tools::RegexConvertOpts::WILDCARD_ALL;
        }
        if (mods.contains("+")) {
            opts |= Tools::RegexConvertOpts::EXACT_MATCH;
            opts |= Tools::RegexConvertOpts::CASE_SENSITIVE;
        }
        term.regex = Tools::convertToRegex(term.word, opts);
        const auto typedWord = term.word;

        // Exclude modifier
        term.exclude = mods.contains("-") || mods.contains("!");

        // Determine the field to search
        term.field = Field::Undefined;

        QString field = result.captured(2);
        if (!field.isEmpty()) {
            if (field.startsWith("_", Qt::CaseInsensitive)) {
                term.field = Field::AttributeValue;
                // searching a custom attribute
                // in this case term.word is the attribute key (removing the leading "_")
                // and term.regex is used to match attribute value
                term.word = field.mid(1);
            } else {
                for (const auto& pair : fieldnames) {
                    if (pair.first.startsWith(field, Qt::CaseInsensitive)) {
                        term.field = pair.second;
                        break;
                    }
                }
            }
        }

        // Alternative spellings: not for exact or regex words, nor for fields whose values are not typed text
        if (m_wordVariants && !mods.contains("+") && !mods.contains("*") && term.field != Field::Password
            && term.field != Field::Is && term.field != Field::Has && term.field != Field::Uuid) {
            for (const auto& variant : m_wordVariants(typedWord)) {
                term.alternatives.append({variant, KeyboardLayoutTranslator::variantRegex(typedWord, variant, opts)});
            }
        }

        m_searchTerms.append(term);
    }
}
