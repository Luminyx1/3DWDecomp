#include "Library/Base/HashCodeUtil.hpp"

#include <cctype>
#include <cstdio>

namespace al {
/**
 * Calculates the hash code of a string.
 * @param pStr The string.
 * @return The hash code.
 */
u32 calcHashCode(const char* pStr) {
    if (pStr[0] == '\0') {
        return 0;
    }

    u32 hashCode = 0;
    for (s32 i = 0; pStr[i] != '\0'; i++) {
        hashCode = (hashCode * 0x1f) + pStr[i];
    }

    return hashCode;
}

/**
 * Calculates the hash code of a string converted to lower case.
 * @param pStr The string.
 * @return The hash code.
 */
u32 calcHashCodeLower(const char* pStr) {
    u32 hashCode = 0;
    for (s32 i = 0; pStr[i] != '\0'; i++) {
        hashCode = (hashCode * 0x1f) + tolower(pStr[i]);
    }

    return hashCode;
}

/**
 * Calculates the hash code of a formatted string.
 * @param pFormat Format string.
 * @param args Format arguments.
 * @return The hash code.
 */
u32 calcHashCodeFmt(const char* pFormat, std::va_list args) {
    char buf[0x100];
    vsnprintf(buf, 0x100, pFormat, args);
    return calcHashCode(buf);
}

/**
 * Calculates the hash code of a formatted string.
 * @param pFormat Format string.
 * @return The hash code.
 */
u32 calcHashCodeFmt(const char* pFormat, ...) {
    std::va_list args;
    va_start(args, pFormat);
    u32 result = calcHashCodeFmt(pFormat, args);
    va_end(args);
    return result;
}

/**
 * Calculates the hash code of a string offset by a pointer value.
 * @param pStr The string.
 * @param pOffset Value added to the hash code.
 * @return The hash code.
 */
u32 calcHashOffsetPtr(const char* pStr, const void* pOffset) {
    return calcHashCode(pStr) + static_cast<u32>(reinterpret_cast<uintptr_t>(pOffset));
}
}  // namespace al
