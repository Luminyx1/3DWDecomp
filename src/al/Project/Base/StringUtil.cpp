#include "Project/Base/StringUtil.hpp"
#include <prim/seadMemUtil.h>
#include <cstring>
#include <strings.h>

namespace al {
    /**
     * @brief Checks whether a pointer points into the current thread's stack.
     * @param pPtr The pointer to check.
     * @return Whether the pointer is on the stack.
     */
    bool isInStack(const void* pPtr) {
        return sead::MemUtil::isStack(pPtr);
    }

    /**
     * @brief Compares two strings for equality.
     * @param pStr1 The first string.
     * @param pStr2 The second string.
     * @return Whether both strings are equal.
     */
    bool isEqualString(const char* pStr1, const char* pStr2) {
        while (*pStr1 == *pStr2) {
            char val = *pStr1;

            if (val == '\0') {
                return true;
            }

            pStr2++;
            pStr1++;
        }

        return false;
    }

    /**
     * @brief Compares two sead strings for equality.
     * @param rStr1 The first string.
     * @param rStr2 The second string.
     * @return Whether both strings are equal.
     */
    bool isEqualString(const sead::SafeString& rStr1, const sead::SafeString& rStr2) {
        const char* str1 = rStr1.cstr();
        const char* str2 = rStr2.cstr();

        while (*str1 == *str2) {
            char val = *str1;

            if (val == '\0') {
                return true;
            }

            str2++;
            str1++;
        }

        return false;
    }

    /**
     * @brief Compares two strings for equality, ignoring case.
     * @param pStr1 The first string.
     * @param pStr2 The second string.
     * @return Whether both strings are equal ignoring case.
     */
    bool isEqualStringCase(const char* pStr1, const char* pStr2) {
        return strcasecmp(pStr1, pStr2) == 0;
    }

    /**
     * @brief Compares two sead strings for equality, ignoring case.
     * @param rStr1 The first string.
     * @param rStr2 The second string.
     * @return Whether both strings are equal ignoring case.
     */
    bool isEqualStringCase(const sead::SafeString& rStr1, const sead::SafeString& rStr2) {
        return strcasecmp(rStr1.cstr(), rStr2.cstr()) == 0;
    }
};
