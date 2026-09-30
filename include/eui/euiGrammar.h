#pragma once

#include <prim/seadEnum.h>

namespace eui {

class Grammar {
public:
    SEAD_ENUM(WordAttrCount, Singular, Plural, Few)
    static WordAttrCount getWordAttrCount(int count);
    static WordAttrCount getWordAttrCount(float count);
    static bool isPatchim(char16_t character, bool excludeRieul);
    static bool isStringEndWithPatchim(const char16_t* pText, u32 length, bool excludeRieul);
};

}  // namespace eui
