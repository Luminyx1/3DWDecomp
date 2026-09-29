#include "Project/Base/HashUtil.hpp"
#include <cctype>
#include <cstdio>

namespace al {
    /**
     * @brief Calculates the hash of a string.
     * @param pStr The string to hash.
     * @return The hash of the string.
     */
    u32 calcHashCode(const char* pStr) {
        u32 hash = 0;
        for (; *pStr != '\0'; pStr++) {
            hash = hash * 31 + static_cast<u8>(*pStr);
        }

        return hash;
    }

    /**
     * @brief Calculates the hash of a string, ignoring case.
     * @param pStr The string to hash.
     * @return The hash of the lowercase string.
     */
    u32 calcHashCodeLower(const char* pStr) {
        u32 hash = 0;
        for (; *pStr != '\0'; pStr++) {
            hash = hash * 31 + tolower(static_cast<u8>(*pStr));
        }

        return hash;
    }

    /**
     * @brief Calculates the hash of a formatted string.
     * @param pFormat The format string.
     * @param args The format arguments.
     * @return The hash of the formatted string.
     */
    u32 calcHashCodeFmt(const char* pFormat, std::va_list args) {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pFormat, args);
        return calcHashCode(buffer);
    }

    /**
     * @brief Calculates the hash of a formatted string.
     * @param pFormat The format string.
     * @return The hash of the formatted string.
     */
    u32 calcHashCodeFmt(const char* pFormat, ...) {
        std::va_list args;
        va_start(args, pFormat);
        u32 hash = calcHashCodeFmt(pFormat, args);
        va_end(args);
        return hash;
    }

    /**
     * @brief Calculates the hash of a string offset by a pointer's address.
     * @param pStr The string to hash.
     * @param pPtr The pointer to offset the hash by.
     * @return The hash of the string plus the pointer's address.
     */
    u32 calcHashOffsetPtr(const char* pStr, const void* pPtr) {
        return calcHashCode(pStr) + static_cast<u32>(reinterpret_cast<uintptr_t>(pPtr));
    }
};
