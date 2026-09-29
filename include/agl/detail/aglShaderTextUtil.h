#pragma once

#include <prim/seadSafeString.h>

namespace sead {
class Heap;
}

namespace agl::detail {

class ShaderTextUtil {
public:
    static sead::HeapSafeString* createRawText(const sead::SafeString& rText,
                                               const char* const* pSourceNames,
                                               const char* const* pSourceTexts, s32 sourceNum,
                                               bool* pUsedFlags, sead::Heap* pHeap);
    static bool isUTF8(const char* pText);
    static void replaceMacro(sead::BufferedSafeString* pText, const char* const* pMacros,
                             const char* const* pValues, s32 macroNum, char* pWork,
                             s32 workSize);
    static const char* findLineFeedCode(const char* pText, s32* pLength);
};

}  // namespace agl::detail
