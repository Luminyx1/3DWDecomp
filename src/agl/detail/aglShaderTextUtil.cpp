#include "detail/aglShaderTextUtil.h"

#include <basis/seadNew.h>
#include <prim/seadMemUtil.h>

namespace agl::detail {

namespace {

inline bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

}  // namespace

void ShaderTextUtil::replaceMacro(sead::BufferedSafeString* pText, const char* const* pMacros,
                                  const char* const* pValues, s32 macroNum, char* pWork,
                                  s32 workSize) {
    bool isReplaced[1024];
    for (s32 i = 0; i < macroNum; i++) {
        isReplaced[i] = false;
    }

    const char* src = pText->cstr();
    char* dst = pWork;
    s32 replacedNum = 0;
    for (;;) {
        s32 lineFeedLength;
        s32 lineLength = findLineFeedCode(src, &lineFeedLength);
        if (lineLength == -1) {
            break;
        }

        if (src[0] == '#') {
            const char* p = src + 1;
            while (isSpace(*p)) {
                p++;
            }
            if (p[0] == 'd' && p[1] == 'e' && p[2] == 'f' && p[3] == 'i' && p[4] == 'n' &&
                p[5] == 'e' && (p[6] == ' ' || p[6] == '\t')) {
                const char* name = p + 7;
                while (isSpace(*name)) {
                    name++;
                }
                for (s32 i = 0; i < macroNum; i++) {
                    if (isReplaced[i]) {
                        continue;
                    }
                    const char* macro = pMacros[i];
                    const char* m = macro;
                    const char* n = name;
                    while (*m != '\0') {
                        if (*n != *m) {
                            goto next;
                        }
                        m++;
                        n++;
                    }
                    if (*n == '\t' || *n == ' ') {
                        sead::BufferedSafeString define(dst, workSize - (dst - pWork));
                        dst += define.format("#define %s %s", macro, pValues[i]);
                        for (s32 j = 0; j < lineFeedLength; j++) {
                            sead::BufferedSafeString lineFeed(dst, workSize - (dst - pWork));
                            dst += lineFeed.append(src[lineLength + j]);
                        }
                        isReplaced[i] = true;
                        src += lineLength + lineFeedLength;
                        goto replaced;
                    }
                next:;
                }
            }
        }

        {
            s32 length = lineLength + lineFeedLength;
            sead::MemUtil::copy(dst, src, length);
            dst += length;
            *dst = '\0';
            src += length;
        }
        continue;

    replaced:
        if (++replacedNum == macroNum) {
            break;
        }
    }

    while (*src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
    pText->copy(pWork);
}

/**
 * Finds the first line feed code in a text.
 * @param pText text to search
 * @param pLength receives the length of the line feed code (1 or 2), may be nullptr
 * @return offset of the line feed code, or -1 if there is none
 */
s32 ShaderTextUtil::findLineFeedCode(const char* pText, s32* pLength) {
    for (s32 i = 0; pText[i] != '\0'; i++) {
        const char* p = pText + i;
        s32 length;
        if (*p == '\n') {
            length = 1;
        } else if (*p == '\r') {
            length = p[1] == '\n' ? 2 : 1;
        } else {
            continue;
        }
        if (pLength) {
            *pLength = length;
        }
        return i;
    }
    return -1;
}

/**
 * Replaces the range [begin, end) of a text with another string.
 * @param pText text to modify in place
 * @param pInsert string to insert
 * @param begin offset of the first replaced character
 * @param end offset after the last replaced character
 * @param pWork work buffer receiving the text after the range
 * @param workSize size of the work buffer
 */
void ShaderTextUtil::replace(char* pText, const char* pInsert, s32 begin, s32 end, void* pWork,
                             s32 workSize) {
    char* work = static_cast<char*>(pWork);
    char* dst = work;
    for (const char* src = pText + end; *src != '\0'; src++) {
        *dst++ = *src;
    }
    *dst = '\0';

    dst = pText + begin;
    for (const char* src = pInsert; *src != '\0'; src++) {
        *dst++ = *src;
    }
    for (const char* src = work; *src != '\0'; src++) {
        *dst++ = *src;
    }
    *dst = '\0';
}

/**
 * Checks whether a text starts with a UTF-8 byte order mark.
 * @param pText text to check
 * @return whether the text starts with a UTF-8 byte order mark
 */
bool ShaderTextUtil::isUTF8(const char* pText) {
    return static_cast<u8>(pText[0]) == 0xef && static_cast<u8>(pText[1]) == 0xbb &&
           static_cast<u8>(pText[2]) == 0xbf;
}

sead::HeapSafeString* ShaderTextUtil::createRawText(const sead::SafeString& rText,
                                                    const char* const* pSourceNames,
                                                    const char* const* pSourceTexts,
                                                    s32 sourceNum, bool* pUsedFlags,
                                                    sead::Heap* pHeap) {
    if (pUsedFlags) {
        for (s32 i = 0; i < sourceNum; i++) {
            pUsedFlags[i] = false;
        }
    }

    sead::HeapSafeString* text = new (pHeap) sead::HeapSafeString(pHeap, rText, 8);
    s32 length = text->calcLength();
    const char* p = text->cstr();
    while (*p != '\0') {
        const char* sharp = p;
        while (*sharp != '#') {
            sharp++;
            if (*sharp == '\0') {
                return text;
            }
        }

        const char* directive = sharp + 1;
        while (isSpace(*directive)) {
            directive++;
        }
        if (*directive == '\0') {
            directive = nullptr;
        }

        if (directive[0] != 'i' || directive[1] != 'n' || directive[2] != 'c' ||
            directive[3] != 'l' || directive[4] != 'u' || directive[5] != 'd' ||
            directive[6] != 'e') {
            p = directive;
            continue;
        }

        const char* nameBegin = directive + 7;
        while (*nameBegin != '"') {
            if (*nameBegin == '\0') {
                break;
            }
            nameBegin++;
        }
        if (*nameBegin == '\0') {
            continue;
        }
        nameBegin++;
        const char* nameEnd = nameBegin;
        while (*nameEnd != '"') {
            if (*nameEnd == '\0') {
                break;
            }
            nameEnd++;
        }
        if (*nameEnd == '\0') {
            continue;
        }
        const char* includeEnd = nameEnd + 1;

        sead::FixedSafeString<1024> name;
        name.copy(nameBegin, includeEnd - nameBegin - 1);

        s32 i = 0;
        for (;; i++) {
            if (i >= sourceNum) {
                return text;
            }
            if (name == sead::SafeString(pSourceNames[i])) {
                break;
            }
        }

        const char* source = pSourceTexts[i];
        if (pUsedFlags) {
            pUsedFlags[i] = true;
        }
        if (!source) {
            return text;
        }
        if (isUTF8(source)) {
            source += 3;
        }
        s32 sourceLength = sead::SafeString(source).calcLength();

        sead::HeapSafeString* newText =
            new (pHeap) sead::HeapSafeString(pHeap, sourceLength + length + 1);
        newText->copy(*text);

        const char* top = text->cstr();
        char* work = new (pHeap, 8) char[length + 1];
        replace(newText->getBuffer(), source, sharp - top, includeEnd - top, work, length + 1);
        delete[] work;

        delete text;
        text = new (pHeap) sead::HeapSafeString(pHeap, *newText, 8);
        delete newText;

        p = text->cstr();
        length = text->calcLength();
    }
    return text;
}

/**
 * Checks whether a character separates tokens in shader source.
 * @param c character to check
 * @return whether the character is a delimiter
 */
bool IsDelimiter(char c) {
    switch (c) {
    case '\0':
    case '\t':
    case '\n':
    case '\r':
    case ' ':
    case '!':
    case '"':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case ';':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '[':
    case '\\':
    case ']':
    case '^':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
        return true;
    default:
        return false;
    }
}

/**
 * Constructs an empty analyze result.
 */
ShaderTextUtil::ShaderDumpTextAnalyzeResult::ShaderDumpTextAnalyzeResult()
    : _0(0), _8(0), _10(0), _18(0), _20(0) {}

/**
 * Advances a text pointer to the first occurrence of a character or the end of the text.
 * @param c character to find
 * @param ppText text pointer to advance
 */
void ShaderTextUtil::findFirstChar(char c, const char** ppText) {
    while (**ppText != c && **ppText != '\0') {
        (*ppText)++;
    }
}

/**
 * Advances a text pointer past all leading occurrences of a character.
 * @param c character to skip
 * @param ppText text pointer to advance
 */
void ShaderTextUtil::skipChar(char c, const char** ppText) {
    while (**ppText == c && **ppText != '\0') {
        (*ppText)++;
    }
}

bool ShaderTextUtil::skipFirstMatchedString(const sead::SafeString& rStr, const char** ppText) {
    s32 length = rStr.calcLength();
    char c = **ppText;
    if (c == '\0') {
        return false;
    }
    do {
        for (s32 i = 0; i < length; i++) {
            if (c == '\0') {
                return false;
            }
            if (rStr.at(i) != c) {
                break;
            }
            (*ppText)++;
            if (i == length - 1) {
                return true;
            }
            c = **ppText;
        }
        c = *++(*ppText);
    } while (c != '\0');
    return false;
}

/**
 * Checks whether a text starts with a string.
 * @param rStr string to match
 * @param pText text to check
 * @return whether the text starts with the string
 */
bool ShaderTextUtil::matchString(const sead::SafeString& rStr, const char* pText) {
    s32 length = rStr.calcLength();
    for (s32 i = 0; i < length; i++, pText++) {
        if (*pText == '\0' || *pText != rStr.at(i)) {
            return false;
        }
        if (i == length - 1) {
            return true;
        }
    }
    return false;
}

/**
 * Advances a text pointer over a string it starts with, stopping on its last character.
 * @param rStr string to skip
 * @param ppText text pointer to advance
 * @return whether the text started with the string
 */
bool ShaderTextUtil::skipString(const sead::SafeString& rStr, const char** ppText) {
    s32 length = rStr.calcLength();
    for (s32 i = 0; i < length; i++) {
        if (**ppText == '\0' || **ppText != rStr.at(i)) {
            return false;
        }
        if (i == length - 1) {
            return true;
        }
        (*ppText)++;
    }
    return false;
}

}  // namespace agl::detail
