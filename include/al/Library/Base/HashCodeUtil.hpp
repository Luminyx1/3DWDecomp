#pragma once

#include <basis/seadTypes.h>
#include <cstdarg>

namespace al {
u32 calcHashCode(const char* pStr);
u32 calcHashCodeLower(const char* pStr);
u32 calcHashCodeFmt(const char* pFormat, std::va_list args);
u32 calcHashCodeFmt(const char* pFormat, ...);
u32 calcHashOffsetPtr(const char* pStr, const void* pOffset);
}  // namespace al
