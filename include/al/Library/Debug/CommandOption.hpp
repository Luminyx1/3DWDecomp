#pragma once

#include <prim/seadSafeString.h>

namespace al {
bool isCommandName(const char* pCommandLine, const char* pName);
bool isExistOption(const char* pCommandLine, const char* pOptionName,
                   sead::BufferedSafeString* pOutValue);
bool tryGetIntOptionValue(s32* pOut, const char* pCommandLine, const char* pOptionName);
bool tryGetFloatOptionValue(f32* pOut, const char* pCommandLine, const char* pOptionName);
bool tryGetBoolOptionValue(bool* pOut, const char* pCommandLine, const char* pOptionName);
bool tryGetStringOptionValue(sead::BufferedSafeString* pOut, const char* pCommandLine,
                             const char* pOptionName);
}  // namespace al
