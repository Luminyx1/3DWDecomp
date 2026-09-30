#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.hpp>

namespace al {
bool isInStack(const void*);
bool isEqualString(const char*, const char*);
bool isEqualString(const sead::SafeString&, const sead::SafeString&);
bool isEqualStringCase(const char*, const char*);
bool isEqualStringCase(const sead::SafeString&, const sead::SafeString&);
bool isEqualSubString(const char*, const char*);

template <s32 Length>
class StringTmp : public sead::FixedSafeString<Length> {
public:
    StringTmp(const char* pStr, ...) : sead::FixedSafeString<Length>() {
        std::va_list va_args;
        va_start(va_args, pStr);
        sead::FixedSafeString<Length>::formatV(pStr, va_args);
        va_end(va_args);
    }

    StringTmp() : sead::FixedSafeString<Length>() {}
};

template <s32 Length>
class WStringTmp : public sead::WFixedSafeString<Length> {
public:
    WStringTmp(const char16_t* pStr, ...) : sead::WFixedSafeString<Length>() {
        std::va_list va_args;
        va_start(va_args, pStr);
        sead::WFixedSafeString<Length>::formatV(pStr, va_args);
        va_end(va_args);
    }

    WStringTmp() : sead::WFixedSafeString<Length>() {}
};

class MatchStr {
public:
    inline MatchStr(const char* pStr) { mStr = pStr; }

    const char* mStr;
};

bool isMatchString(const char*, const MatchStr&);
const char* getSubStringUnmatched(const char** pOutRest, const char* pStr, const MatchStr& rMatch,
                                  void (*pCallback)(const char*, const char*, void*),
                                  void* pUser);
const char* getSubStringUnmatched(const char* pStr, const MatchStr& rMatch);
}  // namespace al
