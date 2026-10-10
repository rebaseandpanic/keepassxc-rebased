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

// Keyboard layouts of macOS, read through the Text Input Sources API and
// translated with UCKeyTranslate.
//
// Which input sources count: the keyboard input sources the user has enabled
// (the ones in the input menu) plus the keyboard layout currently in use, which
// differs from them when an input method (e.g. Japanese or Chinese) is selected
// and types through a layout that is not enabled on its own. Only sources that
// carry Unicode ('uchr') layout data are used: input methods and their modes have
// none, and legacy KCHR-only layouts have none either; both are skipped.
//
// Text Input Sources API calls are not thread safe and must run on the main thread.

#include "KeyboardLayouts.h"

#include <QSet>

#include <array>
#include <memory>
#include <type_traits>

#include <Carbon/Carbon.h>

namespace
{
    struct CFReleaser
    {
        void operator()(CFTypeRef ref) const
        {
            if (ref) {
                ::CFRelease(ref);
            }
        }
    };

    // Owns a CF object obtained under the Create/Copy rule
    template <typename Ref> using CFHolder = std::unique_ptr<std::remove_pointer_t<Ref>, CFReleaser>;

    // Virtual key codes of a keyboard are 0..127
    constexpr UInt16 MaxVirtualKeyCode = 127;
    // UCKeyTranslate produces at most 255 UTF-16 code units
    constexpr UniCharCount MaxKeyOutputLength = 255;

    // Text the key produces with the given modifiers, or an empty string when it
    // produces none, only a dead key, or a control character
    QString translateKey(const UCKeyboardLayout* layout, UInt16 keyCode, UInt32 modifierKeyState, UInt32 keyboardType)
    {
        // A fresh dead-key state for every key: the key is pressed on its own.
        // Dead keys are allowed to set a new state (no kUCKeyTranslateNoDeadKeysMask),
        // so that a dead key is recognised by a non-zero state and empty output
        // instead of being reported as its spacing character.
        UInt32 deadKeyState = 0;
        UniCharCount length = 0;
        std::array<UniChar, MaxKeyOutputLength> buffer{};
        const OSStatus status = ::UCKeyTranslate(layout,
                                                 keyCode,
                                                 kUCKeyActionDown,
                                                 modifierKeyState,
                                                 keyboardType,
                                                 0,
                                                 &deadKeyState,
                                                 buffer.size(),
                                                 &length,
                                                 buffer.data());
        if (status != noErr || length == 0 || deadKeyState != 0) {
            return {};
        }

        const QString text =
            QString::fromUtf16(reinterpret_cast<const char16_t*>(buffer.data()), static_cast<qsizetype>(length));
        for (const QChar ch : text) {
            if (ch.category() == QChar::Other_Control) {
                return {};
            }
        }
        return text;
    }

    // Reads the layout of the input source into result; false when the source
    // carries no Unicode keyboard layout data
    bool readLayout(TISInputSourceRef source, UInt32 keyboardType, KeyboardLayout& result)
    {
        // "Get" rule: the property values are owned by the input source
        const auto layoutData =
            static_cast<CFDataRef>(::TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData));
        if (!layoutData || ::CFDataGetLength(layoutData) == 0) {
            return false;
        }
        const auto* layout =
            static_cast<const UCKeyboardLayout*>(static_cast<const void*>(::CFDataGetBytePtr(layoutData)));

        const auto name = static_cast<CFStringRef>(::TISGetInputSourceProperty(source, kTISPropertyLocalizedName));
        result.name = name ? QString::fromCFString(name) : QString();

        // modifierKeyState is the Event Manager modifiers shifted right by 8 bits
        const UInt32 shiftState = (shiftKey >> 8) & 0xFF;
        for (UInt16 keyCode = 0; keyCode <= MaxVirtualKeyCode; ++keyCode) {
            for (const bool shift : {false, true}) {
                const QString text = translateKey(layout, keyCode, shift ? shiftState : 0, keyboardType);
                if (!text.isEmpty()) {
                    result.keys.insert(KeyboardLayout::keyIndex(keyCode, shift), text);
                }
            }
        }
        return true;
    }
} // namespace

QList<KeyboardLayout> installedKeyboardLayouts()
{
    // Enabled (includeAllInstalled = false) keyboard input sources that can be
    // selected; parent input methods with modes cannot and are left out.
    const void* filterKeys[] = {kTISPropertyInputSourceCategory, kTISPropertyInputSourceIsSelectCapable};
    const void* filterValues[] = {kTISCategoryKeyboardInputSource, kCFBooleanTrue};
    const CFHolder<CFDictionaryRef> filter(::CFDictionaryCreate(kCFAllocatorDefault,
                                                                filterKeys,
                                                                filterValues,
                                                                2,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks));
    if (!filter) {
        return {};
    }

    QList<TISInputSourceRef> sources;
    const CFHolder<CFArrayRef> enabled(::TISCreateInputSourceList(filter.get(), false));
    if (enabled) {
        const CFIndex count = ::CFArrayGetCount(enabled.get());
        for (CFIndex i = 0; i < count; ++i) {
            // "Get" rule: the array keeps the source alive while it is used below
            sources.append(
                static_cast<TISInputSourceRef>(const_cast<void*>(::CFArrayGetValueAtIndex(enabled.get(), i))));
        }
    }
    const CFHolder<TISInputSourceRef> current(::TISCopyCurrentKeyboardLayoutInputSource());
    if (current) {
        sources.append(current.get());
    }

    const UInt32 keyboardType = ::LMGetKbdType();
    QSet<QString> seenIds;
    QList<KeyboardLayout> layouts;
    for (const TISInputSourceRef source : sources) {
        const auto id = static_cast<CFStringRef>(::TISGetInputSourceProperty(source, kTISPropertyInputSourceID));
        const QString sourceId = id ? QString::fromCFString(id) : QString();
        if (!sourceId.isEmpty() && seenIds.contains(sourceId)) {
            continue;
        }

        KeyboardLayout layout;
        if (readLayout(source, keyboardType, layout) && !layout.keys.isEmpty()) {
            if (!sourceId.isEmpty()) {
                seenIds.insert(sourceId);
            }
            layouts.append(layout);
        }
    }
    return layouts;
}

bool keyboardLayoutsSupported()
{
    return true;
}
