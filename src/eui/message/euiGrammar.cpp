#include <eui/euiGrammar.h>

#include <prim/seadEnvUtil.h>

namespace eui {

/**
 * @brief Selects the language-dependent grammatical number category.
 * @param[in] count Integer quantity used by the message.
 * @return Zero for singular, one for plural, or two for the Russian few category.
 */
Grammar::WordAttrCount Grammar::getWordAttrCount(int count) {
    const auto language = sead::EnvUtil::getLanguage();
    if (language == sead::LanguageID::fr)
        return count < -1 || count > 1;
    if (language == sead::LanguageID::ru) {
        const int units = count % 10;
        const int lastTwoDigits = count % 100;
        if (units == 1 && lastTwoDigits != 11)
            return 0;
        if (units == 2 && lastTwoDigits != 12)
            return 2;
        if (units == 3 && lastTwoDigits != 13)
            return 2;
        if (units == 4 && lastTwoDigits != 14)
            return 2;
        return 1;
    }

    if (count == -1)
        return WordAttrCount::Singular;
    if (count == 1)
        return WordAttrCount::Singular;
    return WordAttrCount::Plural;
}

/**
 * @brief Selects the grammatical number category for a floating-point quantity.
 * @param[in] count Quantity used by the message, including a possible fractional part.
 * @return The language's number category; Russian floating-point values use category two.
 */
Grammar::WordAttrCount Grammar::getWordAttrCount(float count) {
    const auto language = sead::EnvUtil::getLanguage();
    if (language == sead::LanguageID::ru)
        return 2;
    if (language == sead::LanguageID::fr)
        return count > -2.0f && count < 2.0f ? 0 : 1;
    return count == -1.0f || count == 1.0f ? 0 : 1;
}

/**
 * @brief Tests a character's final consonant for Korean particle selection.
 * @param[in] character Hangul syllable or ASCII/full-width decimal digit to inspect.
 * @param[in] excludeRieul Treat a final rieul as absent, as required by some particles.
 * @return Whether a final consonant is present; unrecognized characters return true.
 */
bool Grammar::isPatchim(char16_t character, bool excludeRieul) {
    if (static_cast<char16_t>(character - 0xac00) <= 0x2ba3) {
        const int consonant = (character - 0xac00) % 28;
        return (consonant != 0) ^ (excludeRieul && consonant == 8);
    }

    if (static_cast<char16_t>(character - u'0') < 10 ||
        static_cast<char16_t>(character - u'０') < 10) {
        const int digit = character < u':' ? character - u'0' : character - u'０';
        if (excludeRieul) {
            switch (digit) {
            case 0: case 3: case 6: return true;
            default: return false;
            }
        }

        switch (digit) {
        case 2: case 4: case 5: case 9: return false;
        default: return true;
        }
    }

    return true;
}

/**
 * @brief Tests the last character other than spaces or line breaks, skipping message tags.
 * @param[in] pText UTF-16 message text containing well-formed control tags.
 * @param[in] length Number of UTF-16 code units to scan.
 * @param[in] excludeRieul Treat a final rieul as absent for particle selection.
 * @return Whether the final visible character has patchim; false for empty text.
 */
bool Grammar::isStringEndWithPatchim(const char16_t* pText, u32 length, bool excludeRieul) {
    if (length == 0)
        return false;
    const char16_t* pEnd = pText + length;
    char16_t last = 0;
    while (pText < pEnd) {
        const char16_t character = *pText;
        // Both the start tag (0x0e) and end tag (0x0f) satisfy this test.
        if ((character | 1) == 0xf) {
            if (character == 0xe)
                pText = reinterpret_cast<const char16_t*>(
                    reinterpret_cast<const char*>(pText) + pText[3] + 8);
            else if (character == 0xf)
                pText += 3;
        } else {
            switch (character) {
            case u'\n': case u'\r': case u' ':
                ++pText;
                continue;
            }

            if (character != u'　')
                last = character;
            ++pText;
        }
    }

    return last != 0 && isPatchim(last, excludeRieul);
}

}  // namespace eui
