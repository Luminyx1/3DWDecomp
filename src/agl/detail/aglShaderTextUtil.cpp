#include "detail/aglShaderTextUtil.h"

#include <basis/seadNew.h>
#include <prim/seadMemUtil.h>

namespace agl::detail {

namespace {

inline bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline const char* findChar(const char* p, char c) {
    while (*p != '\0') {
        if (*p == c) {
            return p;
        }

        p++;
    }

    return nullptr;
}

inline const char* skipSpace(const char* p) {
    while (isSpace(*p)) {
        p++;
    }

    return *p == '\0' ? nullptr : p;
}

}  // namespace

// NON_MATCHING: register allocation of the line feed length and the matched macro index
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
        s32 lineFeedPos;
        s32 i;
        const char* macro;
        for (;;) {
            lineFeedPos = findLineFeedCode(src, &lineFeedLength);
            if (lineFeedPos == -1) {
                goto end;
            }

            if (*src == '#') {
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

                    for (i = 0; i < macroNum; i++) {
                        if (isReplaced[i]) {
                            continue;
                        }

                        macro = pMacros[i];
                        bool match = true;
                        s32 j = 0;
                        for (; macro[j] != '\0'; j++) {
                            if (name[j] != macro[j]) {
                                match = false;
                                break;
                            }
                        }

                        if (match && (name[j] == ' ' || name[j] == '\t')) {
                            break;
                        }
                    }

                    if (i < macroNum) {
                        break;
                    }
                }
            }

            sead::MemUtil::copy(dst, src, lineFeedPos + lineFeedLength);
            dst += lineFeedPos + lineFeedLength;
            *dst = '\0';
            src += lineFeedPos + lineFeedLength;
        }

        dst += sead::BufferedSafeString(dst, workSize - s32(dst - pWork))
                   .format("#define %s %s", macro, pValues[i]);

        for (s32 k = 0; k < lineFeedLength; k++) {
            dst += sead::BufferedSafeString(dst, workSize - s32(dst - pWork))
                       .append(src[lineFeedPos + k]);
        }

        isReplaced[i] = true;
        src += lineFeedPos + lineFeedLength;

        replacedNum++;
        if (replacedNum == macroNum) {
            break;
        }
    }

end:
    while (*src != '\0') {
        *dst++ = *src++;
    }

    *dst = '\0';

    pText->copy(sead::SafeString(pWork));
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

// NON_MATCHING: include search loop structure and register allocation
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

    sead::HeapSafeString* text = new (pHeap) sead::HeapSafeString(pHeap, rText);
    s32 length = text->calcLength();
    const char* src = text->cstr();

    while (*src != '\0') {
        const char* const directiveBegin = findChar(src, '#') + 1;
        if (directiveBegin - 1 == nullptr) {
            break;
        }

        const char* directive = skipSpace(directiveBegin);
        if (directive[0] == 'i' && directive[1] == 'n' && directive[2] == 'c' &&
            directive[3] == 'l' && directive[4] == 'u' && directive[5] == 'd' &&
            directive[6] == 'e') {
            const char* const nameBegin = findChar(directive + 7, '"') + 1;
            if (nameBegin - 1 == nullptr) {
                continue;
            }

            const char* const includeEnd = findChar(nameBegin, '"') + 1;
            if (includeEnd - 1 == nullptr) {
                continue;
            }

            sead::FixedSafeString<1024> name;
            name.copy(nameBegin, s32(includeEnd - nameBegin) - 1);

            s32 i = 0;
            for (; i < sourceNum; i++) {
                if (name.isEqual(pSourceNames[i])) {
                    break;
                }
            }

            if (i >= sourceNum) {
                break;
            }

            const char* source = pSourceTexts[i];
            if (pUsedFlags) {
                pUsedFlags[i] = true;
            }

            if (!source) {
                break;
            }

            if (isUTF8(source)) {
                source += 3;
            }

            const s32 sourceLength = sead::SafeString(source).calcLength();
            sead::HeapSafeString* newText =
                new (pHeap) sead::HeapSafeString(pHeap, sourceLength + length + 1);
            newText->copy(*text);

            const char* top = text->cstr();
            char* work = new (pHeap) char[length + 1];
            replace(const_cast<char*>(newText->cstr()), source, s32(directiveBegin - top) - 1,
                    s32(includeEnd - top), work, length + 1);
            delete[] work;

            delete text;
            text = new (pHeap) sead::HeapSafeString(pHeap, *newText);
            delete newText;

            src = text->cstr();
            length = text->calcLength();
        } else {
            src = directive;
        }
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
    : mAluClauseInstNum(0), mTexClauseInstNum(0), mExportNum(0), mVaryingInNum(0),
      mVaryingOutNum(0), mDisassembly(nullptr), mDisassemblySize(0) {}

// NON_MATCHING: clause loop layout and register allocation in the symbol section
void ShaderTextUtil::analyzeShaderDumpText(const sead::SafeString& rText,
                                           ShaderDumpTextAnalyzeResult* pResult) {
    ShaderDumpTextAnalyzeResult result;

    const char* p = "";
    for (const char* text = rText.cstr(); *text != '\0'; text++) {
        if (text[0] == ';' && text[1] == ' ' && text[2] == '-' && text[3] == '-') {
            p = text;
            result.mDisassembly = text;
        }
    }

    s32 exportNum = 0;
    s32 clauseNum = 0;
    s32* counter = nullptr;
    while (*p != '\0') {
        if (p[0] == 'E' && clauseNum > 0) {
            if (p[1] == 'N' && p[2] == 'D' && p[3] == '_' && p[4] == 'O' && p[5] == 'F') {
                while (*p != '\n') {
                    p++;
                }

                result.mDisassemblySize = p - result.mDisassembly;
                break;
            }
        } else if ('0' <= *p && *p <= '9') {
            s32 digit[256];
            s32 digitNum;
            if (clauseNum < 100) {
                digit[0] = clauseNum / 10;
                digit[1] = clauseNum - digit[0] * 10;
                digitNum = 2;
            } else if (clauseNum < 1000) {
                digit[0] = clauseNum / 100;
                digit[1] = (clauseNum - digit[0] * 100) / 10;
                digit[2] = clauseNum - digit[0] * 100 - digit[1] * 10;
                digitNum = 3;
            } else {
                digit[0] = clauseNum / 1000;
                digit[1] = (clauseNum - digit[0] * 1000) / 100;
                digit[2] = clauseNum - digit[0] * 1000 - digit[1] * 100;
                digit[3] = clauseNum - digit[0] * 1000 - digit[1] * 100 - digit[2] * 10;
                digitNum = 4;
            }

            for (s32 i = 0; digit[i] == *p - '0'; p++) {
                if (++i >= digitNum) {
                    const char* type = p + 2;
                    if (type[0] == 'A' && type[1] == 'L' && type[2] == 'U') {
                        counter = &result.mAluClauseInstNum;
                    } else if (type[0] == 'E' && type[1] == 'X' && type[2] == 'P') {
                        exportNum++;
                    } else if (type[0] == 'T' && type[1] == 'E' && type[2] == 'X') {
                        counter = &result.mTexClauseInstNum;
                    } else {
                        counter = nullptr;
                    }

                    clauseNum++;
                    break;
                }
            }
        } else if (*p == ' ') {
            skipChar(' ', &p);
            if ('0' <= *p && *p <= '9') {
                while ('0' <= *p && *p <= '9') {
                    p++;
                }

                if (counter) {
                    (*counter)++;
                }
            }
        }

        char c;
        do {
            c = *p++;
        } while (c != '\0' && c != '\n');
        if (c != '\n') {
            p--;
        }
    }

    const sead::SafeString name = "Name: ";
    const sead::SafeString symbolType = "Symbol Type: ";
    const sead::SafeString dataType = "Data Type :";
    const sead::SafeString attrib = "ATTRIB";
    const sead::SafeString uniformBlock = "UNIFORM_BLOCK";
    const sead::SafeString uniform = "UNIFORM";
    const sead::SafeString varyingIn = "VARYING IN";
    const sead::SafeString varyingOut = "VARYING OUT";
    const sead::SafeString samplerImage = "SAMPLER_IMAGE";

    findFirstChar('-', &p);
    skipChar('-', &p);
    skipFirstMatchedString("Symbol Section ", &p);
    skipChar('-', &p);

    s32 varyingInNum = 0;
    s32 varyingOutNum = 0;
    while (*p != '\0') {
        skipChar(' ', &p);
        skipFirstMatchedString(name, &p);
        skipFirstMatchedString(symbolType, &p);
        if (matchString(attrib, p)) {
            skipString(attrib, &p);
        } else if (matchString(uniformBlock, p)) {
            skipString(uniformBlock, &p);
        } else if (matchString(uniform, p)) {
            skipString(uniform, &p);
            skipFirstMatchedString(dataType, &p);
            if (matchString(samplerImage, p)) {
                skipString(samplerImage, &p);
            }
        } else if (matchString(varyingIn, p)) {
            skipString(varyingIn, &p);
            varyingInNum++;
        } else if (matchString(varyingOut, p)) {
            skipString(varyingOut, &p);
            varyingOutNum++;
        }

        skipChar(' ', &p);
    }

    result.mExportNum = exportNum;
    result.mVaryingInNum = varyingInNum;
    result.mVaryingOutNum = varyingOutNum;
    *pResult = result;
}

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
