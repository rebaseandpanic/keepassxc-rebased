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

#include "TestKeyboardLayoutTranslator.h"

#include "core/Tools.h"
#include "rebased/FakeKeyboardLayouts.h"
#include "rebased/KeyboardLayoutTranslator.h"

#include <QTest>

QTEST_GUILESS_MAIN(TestKeyboardLayoutTranslator)

namespace
{
    // Search wildcards and the space, as the search bar passes them
    const QString PassThrough = QStringLiteral("*?| ");

    KeyboardLayout layoutOf(const QString& name, const QList<QPair<quint32, QString>>& keys)
    {
        KeyboardLayout layout;
        layout.name = name;
        for (const auto& key : keys) {
            layout.keys.insert(key.first, key.second);
        }
        return layout;
    }
} // namespace

void TestKeyboardLayoutTranslator::testVariants_data()
{
    QTest::addColumn<QString>("word");
    QTest::addColumn<QStringList>("variants");

    QTest::newRow("russian keys typed for twitter") << "ецшееук" << QStringList{"twitter"};
    QTest::newRow("english word retyped into russian") << "twitter" << QStringList{"ецшееук"};
    QTest::newRow("russian keys typed for search") << "ыуфкср" << QStringList{"search"};
    QTest::newRow("shift level") << "Ецшееук" << QStringList{"Twitter"};
    QTest::newRow("all shifted") << "ЕЦШЕЕУК" << QStringList{"TWITTER"};
    QTest::newRow("punctuation keys") << "ёлка" << QStringList{"`krf"};
    QTest::newRow("russian letters on english punctuation keys") << "[ktm" << QStringList{"хлеь"};
    QTest::newRow("digits kept") << "twitter2" << QStringList{"ецшееук2"};
    QTest::newRow("shifted digit differs") << "a@b" << QStringList{"ф\"и"};
    QTest::newRow("shifted 7 becomes question mark") << "a&b" << QStringList{"ф?и"};
    QTest::newRow("russian slash on the backslash key") << "ф/и" << QStringList{"a|b"};
    QTest::newRow("same text in every layout") << "12345" << QStringList{};
    QTest::newRow("mixed layouts") << "twitterй" << QStringList{};
    QTest::newRow("character on no key") << "tw€" << QStringList{};
    QTest::newRow("wildcard passes through") << "tw?tter" << QStringList{"ец?ееук"};
    QTest::newRow("wildcards and space pass through") << "tw*t ter|x" << QStringList{"ец*е еук|ч"};
    QTest::newRow("only pass-through characters") << "**" << QStringList{};
    QTest::newRow("empty word") << "" << QStringList{};
}

void TestKeyboardLayoutTranslator::testVariants()
{
    QFETCH(QString, word);
    QFETCH(QStringList, variants);

    const KeyboardLayoutTranslator translator({fakeEnglishLayout(), fakeRussianLayout()}, 2, PassThrough);
    QCOMPARE(translator.variants(word), variants);
}

void TestKeyboardLayoutTranslator::testMinimumLength_data()
{
    QTest::addColumn<int>("minimumLength");
    QTest::addColumn<QString>("word");
    QTest::addColumn<QStringList>("variants");

    QTest::newRow("shorter than minimum") << 2 << "е" << QStringList{};
    QTest::newRow("equal to minimum") << 2 << "ец" << QStringList{"tw"};
    QTest::newRow("minimum one") << 1 << "е" << QStringList{"t"};
    QTest::newRow("minimum ten, nine letters") << 10 << "ецшееукец" << QStringList{};
    QTest::newRow("minimum ten, ten letters") << 10 << "ецшееукецш" << QStringList{"twittertwi"};
}

void TestKeyboardLayoutTranslator::testMinimumLength()
{
    QFETCH(int, minimumLength);
    QFETCH(QString, word);
    QFETCH(QStringList, variants);

    const KeyboardLayoutTranslator translator({fakeEnglishLayout(), fakeRussianLayout()}, minimumLength, PassThrough);
    QCOMPARE(translator.variants(word), variants);
}

void TestKeyboardLayoutTranslator::testSingleLayout()
{
    const KeyboardLayoutTranslator translator({fakeRussianLayout()}, 2, PassThrough);
    QCOMPARE(translator.variants("ецшееук"), QStringList{});

    const KeyboardLayoutTranslator noLayouts({}, 2, PassThrough);
    QCOMPARE(noLayouts.variants("ецшееук"), QStringList{});
}

void TestKeyboardLayoutTranslator::testDuplicateVariantsAcrossLayouts()
{
    // A second English layout yields the same retyped words, which are listed once
    auto english2 = fakeEnglishLayout();
    english2.name = "English 2";
    const KeyboardLayoutTranslator translator({fakeEnglishLayout(), fakeRussianLayout(), english2}, 2, PassThrough);

    QCOMPARE(translator.variants("ецшееук"), QStringList{"twitter"});
    QCOMPARE(translator.variants("twitter"), QStringList{"ецшееук"});
}

void TestKeyboardLayoutTranslator::testFirstKeyWinsForRepeatedCharacter()
{
    // "x" is on keys 4 and 2 of layout A; the key with the lower index is used
    const auto a = layoutOf("A", {{4, "x"}, {2, "x"}, {6, "y"}});
    const auto b = layoutOf("B", {{2, "m"}, {4, "n"}, {6, "o"}});
    const KeyboardLayoutTranslator translator({a, b}, 2);

    QCOMPARE(translator.variants("xy"), QStringList{"mo"});
}

void TestKeyboardLayoutTranslator::testKeysWithoutSingleCharacter()
{
    // Keys with empty or multi-character text take no part in the mapping
    const auto a = layoutOf("A", {{2, "ab"}, {4, ""}, {6, "q"}, {8, "w"}, {10, QString::fromUtf8("\xF0\x9F\x98\x80")}});
    const auto b = layoutOf("B", {{2, "c"}, {4, "d"}, {6, "zz"}, {8, "e"}, {10, "s"}});
    const KeyboardLayoutTranslator translator({a, b}, 2);

    // "a" and "b" are no key's whole text in A
    QCOMPARE(translator.variants("ab"), QStringList{});
    // "q" lands on a key of B producing two characters: no variant from A to B
    QCOMPARE(translator.variants("qw"), QStringList{});
    QCOMPARE(translator.variants("ww"), QStringList{"ee"});
    // A character outside the BMP counts as one character
    QCOMPARE(translator.variants(QString::fromUtf8("\xF0\x9F\x98\x80\xF0\x9F\x98\x80")), QStringList{"ss"});
    QCOMPARE(translator.variants(QString::fromUtf8("\xF0\x9F\x98\x80")), QStringList{});
}

void TestKeyboardLayoutTranslator::testVariantRegex_data()
{
    QTest::addColumn<QString>("word");
    QTest::addColumn<QString>("variant");
    QTest::addColumn<int>("opts");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("matches");

    const int wildcards = Tools::RegexConvertOpts::WILDCARD_ALL;
    const int caseSensitive = wildcards | Tools::RegexConvertOpts::CASE_SENSITIVE;

    QTest::newRow("produced ? literal") << "a&b"
                                        << "ф?и" << wildcards << "фхи" << false;
    QTest::newRow("produced ? matches itself") << "a&b"
                                               << "ф?и" << wildcards << "xф?иx" << true;
    QTest::newRow("typed ? wildcard") << "ф?и"
                                      << "a?b" << wildcards << "axb" << true;
    QTest::newRow("typed * wildcard") << "ец*ук"
                                      << "tw*er" << wildcards << "twitter" << true;
    QTest::newRow("produced * literal") << "axb"
                                        << "a*b" << wildcards << "a-long-b" << false;
    QTest::newRow("typed | alternation") << "ец|ук"
                                         << "tw|er" << wildcards << "xxer" << true;
    QTest::newRow("produced | literal") << "ф/и"
                                        << "a|b" << wildcards << "alpha" << false;
    QTest::newRow("regex characters literal") << "ab"
                                              << "(." << wildcards << "xy" << false;
    QTest::newRow("typed ? without wildcard option") << "ф?и"
                                                     << "a?b" << 0 << "axb" << false;
    QTest::newRow("case insensitive") << "ец"
                                      << "tw" << wildcards << "TW" << true;
    QTest::newRow("case sensitive") << "ец"
                                    << "tw" << caseSensitive << "TW" << false;
    QTest::newRow("exact") << "ец"
                           << "tw" << (wildcards | Tools::RegexConvertOpts::EXACT_MATCH) << "twx" << false;
    QTest::newRow("other length, all literal") << "ab?"
                                               << "x?" << wildcards << "xy" << false;
}

void TestKeyboardLayoutTranslator::testVariantRegex()
{
    QFETCH(QString, word);
    QFETCH(QString, variant);
    QFETCH(int, opts);
    QFETCH(QString, text);
    QFETCH(bool, matches);

    QCOMPARE(KeyboardLayoutTranslator::variantRegex(word, variant, opts).match(text).hasMatch(), matches);
}
