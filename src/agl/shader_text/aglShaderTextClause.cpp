#include "shader_text/aglShaderTextClause.h"

#include <basis/seadNew.h>

#include <cmath>
#include <cstdlib>

#include "detail/aglShaderTextUtil.h"

namespace agl {

namespace shtxt {

namespace {

Clause* cloneOne(sead::Heap* pHeap, const Clause& rSrc) {
    Clause* clause = new (pHeap) Clause(rSrc.getType(), rSrc.mBegin, rSrc.mEnd);
    clause->mFlag = rSrc.mFlag;
    return clause;
}

}  // namespace

const Clause::CharacterInfo Clause::cCharacterTable[cType_End + 1] = {
    {cType_None, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_Space, true, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_LineFeed, true, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_LineComment, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_BlockComment, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_BlockCommentLF, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_Int, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_Hex, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_Oct, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_Real, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_Word, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {cType_LParen, false, false, false, false, false, false, false, false, false, false, false, true, false, true, false, false},
    {cType_RParen, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false},
    {cType_LBracket, false, false, false, false, false, false, false, false, false, false, false, true, false, true, false, false},
    {cType_RBracket, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false},
    {cType_LBrace, false, false, false, false, false, false, false, false, false, false, false, true, false, true, false, false},
    {cType_RBrace, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false},
    {cType_Dot, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_Comma, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_Colon, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_Semicolon, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_SingleQuote, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_DoubleQuote, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_Sharp, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_BackSlash, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_Arrow, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
    {cType_Plus, false, false, false, true, false, true, true, false, false, false, false, false, false, false, false, false},
    {cType_Minus, false, false, false, true, false, true, true, false, false, false, false, false, false, false, false, false},
    {cType_Mul, false, false, false, true, false, true, true, false, false, false, false, false, false, false, false, false},
    {cType_Div, false, false, false, true, false, true, true, false, false, false, false, false, false, false, false, false},
    {cType_Mod, false, false, false, true, false, true, true, false, false, false, false, false, false, false, false, false},
    {cType_Assign, false, false, false, true, true, false, true, false, false, true, false, false, false, false, false, false},
    {cType_Tilde, false, false, false, true, true, false, false, true, false, false, false, false, false, false, false, false},
    {cType_And, false, false, false, true, false, true, false, true, false, false, false, false, false, false, false, false},
    {cType_Or, false, false, false, true, false, true, false, true, false, false, false, false, false, false, false, false},
    {cType_Xor, false, false, false, true, false, true, false, true, false, false, false, false, false, false, false, false},
    {cType_ShiftL, false, false, false, true, false, true, false, true, false, false, false, false, false, false, false, false},
    {cType_ShiftR, false, false, false, true, false, true, false, true, false, false, false, false, false, false, false, false},
    {cType_Less, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_Greater, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_LogicalAnd, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_LogicalOr, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_LessEqual, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_GreaterEqual, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_NotEqual, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_Equal, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
    {cType_PlusAssign, false, false, false, true, true, false, true, false, false, true, false, false, false, false, false, false},
    {cType_MinusAssign, false, false, false, true, true, false, true, false, false, true, false, false, false, false, false, false},
    {cType_MulAssign, false, false, false, true, true, false, true, false, false, true, false, false, false, false, false, false},
    {cType_DivAssign, false, false, false, true, true, false, true, false, false, true, false, false, false, false, false, false},
    {cType_ModAssign, false, false, false, true, true, false, true, false, false, true, false, false, false, false, false, false},
    {cType_AndAssign, false, false, false, true, true, false, false, true, false, true, false, false, false, false, false, false},
    {cType_OrAssign, false, false, false, true, true, false, false, true, false, true, false, false, false, false, false, false},
    {cType_XorAssign, false, false, false, true, true, false, false, true, false, true, false, false, false, false, false, false},
    {cType_ShiftLAssign, false, false, false, true, true, false, false, true, false, true, false, false, false, false, false, false},
    {cType_ShiftRAssign, false, false, false, true, true, false, false, true, false, true, false, false, false, false, false, false},
    {cType_Increment, false, false, false, true, true, false, true, false, false, true, true, false, false, false, false, false},
    {cType_Decrement, false, false, false, true, true, false, true, false, false, true, true, false, false, false, false, false},
    {cType_Not, false, false, false, true, true, false, false, false, true, false, true, false, false, false, false, false},
    {cType_Question, false, false, false, true, false, false, false, false, true, false, true, false, false, false, false, false},
    {cType_DoubleSharp, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false},
    {cType_BackQuote, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false},
    {cType_Dollar, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false},
    {cType_At, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false},
    {cType_End, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
};

bool Clause::cTableChecked = false;
u32 Clause::cHashTable[256];

/**
 * Builds the CRC32 table used for clause hashes the first time it is needed.
 */
Clause::TableChecker::TableChecker() {
    if (cTableChecked) {
        return;
    }

    cTableChecked = true;

    for (u32 i = 0; i < 256; i++) {
        u32 value = i;

        for (s32 bit = 0; bit < 8; bit++) {
            value = (value & 1) ? (value >> 1) ^ 0xedb88320 : value >> 1;
        }

        cHashTable[i] = value;
    }
}

/**
 * Constructs an empty clause.
 */
Clause::Clause() : mType(cType_None), mFlag(0), mBegin(nullptr), mEnd(nullptr) {}

/**
 * Constructs a clause covering a text range.
 * @param type clause type
 * @param pBegin start of the text
 * @param pEnd end of the text
 */
Clause::Clause(Type type, const char* pBegin, const char* pEnd)
    : mType(type), mFlag(0), mBegin(pBegin), mEnd(pEnd) {}

/**
 * Destroys the clause.
 */
Clause::~Clause() {}

/**
 * Copies a range of clauses into a new list.
 * @param pHeap heap for the copies
 * @param rBegin first clause to copy
 * @param rEnd clause at which copying stops
 * @param includeEnd whether rEnd itself is copied too
 * @return first clause of the new list, or nullptr if nothing was copied
 */
Clause* Clause::clone(sead::Heap* pHeap, const constIterator& rBegin, const constIterator& rEnd,
                      bool includeEnd) {
    if (rBegin == rEnd) {
        if (!includeEnd) {
            return nullptr;
        }

        return cloneOne(pHeap, *rBegin);
    }

    Clause* head = cloneOne(pHeap, *rBegin);
    Clause* last = head;
    constIterator it = rBegin;

    for (++it; it != rEnd; ++it) {
        Clause* clause = cloneOne(pHeap, *it);
        last->insertAfter(clause);
        last = clause;
    }

    if (includeEnd) {
        last->insertAfter(cloneOne(pHeap, *it));
    }

    return head;
}

/**
 * Scans a numeric literal.
 * @param pType receives the literal type, or cType_None if the text is not a number
 * @param pText text to scan
 * @return end of the literal, or pText if it is not a number
 */
const char* Clause::findNumberBlock(Type* pType, const char* pText) {
    *pType = cType_None;
    const char* p = pText;
    bool isReal = false;
    bool hasExponent;

    if ('1' <= *p && *p <= '9') {
    decimal:
        hasExponent = false;

        while (!detail::IsNumberDelimiter(*++p)) {
            switch (*p) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                break;
            case 'L':
            case 'U':
            case 'l':
            case 'u':
                if (isReal || !detail::IsNumberDelimiter(p[1])) {
                    return pText;
                }

                break;
            case 'E':
            case 'e':
                if (hasExponent) {
                    return pText;
                }

                if (p[1] == '-' || p[1] == '+') {
                    p++;
                }

                hasExponent = true;
                isReal = true;
                break;
            case 'F':
            case 'f':
                if (!isReal || !detail::IsNumberDelimiter(p[1])) {
                    return pText;
                }

                break;
            case '.':
                if (isReal) {
                    return pText;
                }

                isReal = true;
                break;
            default:
                return pText;
            }
        }

        *pType = isReal ? cType_Real : cType_Int;
        return p;
    }

    if (*p == '.') {
        if (!('0' <= p[1] && p[1] <= '9')) {
            return pText;
        }

        isReal = true;
        goto decimal;
    }

    if (*p == '0') {
        if ('0' <= p[1] && p[1] <= '9') {
            while (!detail::IsNumberDelimiter(*++p)) {
                switch (*p) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    break;
                case 'L':
                case 'U':
                case 'l':
                case 'u':
                    if (!detail::IsNumberDelimiter(p[1])) {
                        return pText;
                    }

                    break;
                default:
                    return pText;
                }
            }

            *pType = cType_Oct;
            return p;
        }

        if ((p[1] | 0x20) == 'x') {
            for (p += 2; !detail::IsNumberDelimiter(*p); p++) {
                switch (*p) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    break;
                case 'L':
                case 'U':
                case 'l':
                case 'u':
                    if (!detail::IsNumberDelimiter(p[1])) {
                        return pText;
                    }

                    break;
                default:
                    return pText;
                }
            }

            *pType = cType_Hex;
            return p;
        }

        goto decimal;
    }

    return pText;
}

}  // namespace shtxt

namespace detail {

/**
 * Checks whether a character terminates a numeric literal.
 * @param c character to check
 * @return whether c is a delimiter
 */
bool IsNumberDelimiter(char c) {
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
    case '_':
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

}  // namespace detail

namespace shtxt {

/**
 * Sets the type and text range of the clause.
 * @param type clause type
 * @param pBegin start of the text
 * @param pEnd end of the text
 */
void Clause::set(Type type, const char* pBegin, const char* pEnd) {
    mType = type;
    mBegin = pBegin;
    mEnd = pEnd;
}

/**
 * Appends the clause text to the end of a string.
 * @param pDst destination string
 * @return length of the clause text
 */
s32 Clause::appendTo(sead::BufferedSafeString* pDst) const {
    return appendTo(pDst, pDst->calcLength());
}

/**
 * Appends the clause text to a string starting at a position.
 * @param pDst destination string
 * @param at position in pDst where the appended part starts
 * @return length of the clause text
 */
s32 Clause::appendTo(sead::BufferedSafeString* pDst, u32 at) const {
    sead::BufferedSafeString buffer(pDst->getBuffer() + at, pDst->getBufferSize() - at);
    s32 length = mEnd - mBegin;
    buffer.append(sead::SafeString(mBegin), length);
    return length;
}

/**
 * Copies the clause text into a string, followed by an extra terminator.
 * @param pDst destination string
 * @return length of the clause text plus one
 */
s32 Clause::copyTo(sead::BufferedSafeString* pDst) const {
    s32 length = mEnd - mBegin;
    pDst->copy(sead::SafeString(mBegin), length);
    pDst->append('\0');
    return length + 1;
}

/**
 * Converts the clause text to a number.
 * @return the numeric value
 */
f64 Clause::toNumber() const {
    f64 value = 0.0;
    detail::TextToReal(&value, mBegin, nullptr);
    return value;
}

/**
 * Converts the clause text to a number regardless of the clause type.
 * @return the numeric value
 */
f64 Clause::forceNumber() const {
    f64 value = 0.0;
    detail::TextToReal(&value, mBegin, nullptr);
    return value;
}

}  // namespace shtxt

namespace detail {

/**
 * Parses a C numeric literal (decimal, octal, hexadecimal, binary or floating point).
 * @param pValue receives the value
 * @param pText text to parse
 * @param pIsReal receives whether the literal is a floating point number (may be nullptr)
 * @return end of the literal, or nullptr if the text is not a number
 */
const char* TextToReal(f64* pValue, const char* pText, bool* pIsReal) {
    bool isNegative = false;
    bool isReal = false;
    bool hasDot = false;
    const char* p = pText;
    const char* result = nullptr;

    switch (*p) {
    case '+':
    case '-':
        isNegative = *p == '-';

        if ('0' <= p[1] && p[1] <= '9') {
            p++;
        } else if (p[1] == '.' && '0' <= p[2] && p[2] <= '9') {
            p++;
        } else {
            goto end;
        }

        break;
    case '.':
        p++;

        if (!('0' <= *p && *p <= '9')) {
            goto end;
        }

        isReal = true;
        hasDot = true;
        break;
    default:
        break;
    }

    if (*p == '0') {
        u32 value = 0;
        result = p + 1;

        switch (p[1]) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            while ((*result & 0xf8) == '0') {
                value = value * 8 + (*result - '0');
                result++;
            }

            break;
        case 'X':
        case 'x':
            for (result = p + 2;; result++) {
                s32 digit;
                char c = *result;

                if ('0' <= c && c <= '9') {
                    digit = c - '0';
                } else if ('A' <= c && c <= 'F') {
                    digit = c - 'A' + 10;
                } else if ('a' <= c && c <= 'f') {
                    digit = c - 'a' + 10;
                } else {
                    break;
                }

                value = value * 16 + digit;
            }

            break;
        case 'B':
        case 'b':
            result = p + 2;

            while ((*result & 0xfe) == '0') {
                value = value * 2 + (*result - '0');
                result++;
            }

            break;
        case '.':
            goto decimal;
        default:
            break;
        }

        *pValue = value;
        goto end;
    }

decimal: {
    const char* q = p;

    while ('0' <= *q && *q <= '9') {
        q++;
    }

    bool useStrtod;

    if (*q == '.') {
        hasDot = true;
        useStrtod = true;
    } else if (*q == 'e' || *q == 'E') {
        useStrtod = (q[1] == '-' || q[1] == '+') && '0' <= q[2] && q[2] <= '9';
    } else {
        useStrtod = isReal;
    }

    const char* next;
    bool isFloat;

    if (useStrtod) {
        char* strEnd;
        *pValue = std::strtod(p, &strEnd);
        next = strEnd;
        isFloat = true;
    } else {
        u32 value = 0;
        next = p;

        while ('0' <= *next && *next <= '9') {
            value = value * 10 + (*next - '0');
            next++;
        }

        *pValue = value;
        isFloat = false;
    }

    f64 value = *pValue;
    isReal = isFloat;
    result = p;

    if (std::fabs(value) > __DBL_MAX__ || std::isnan(value)) {
        goto end;
    }

    result = next;

    if (hasDot && (*next | 0x20) == 'f') {
        result = next + 1;
    }
}

end:
    if (pIsReal != nullptr) {
        *pIsReal = isReal;
    }

    if (isNegative) {
        *pValue = -*pValue;
    }

    return result;
}

}  // namespace detail

namespace shtxt {

/**
 * Counts the line feeds in the clause text.
 * @return number of line feeds
 */
s32 Clause::calcLineFeedCount() const {
    s32 count = 0;

    for (const char* p = mBegin; p != mEnd; p++) {
        if (*p == '\n') {
            count++;
        }
    }

    return count;
}

/**
 * Removes characters from the end of the clause text.
 * @param num number of characters to remove
 */
void Clause::chop(u32 num) {
    mEnd -= num;
}

/**
 * Compares the text of two clause sequences, joining adjacent word clauses.
 * @param rOther clause to compare against
 * @param offset number of characters of rOther already consumed
 * @param pEnd end of this sequence
 * @param pOtherEnd end of the other sequence
 * @return comparison result
 */
s32 Clause::compareImpl(const Clause& rOther, u32 offset, const Clause* pEnd,
                        const Clause* pOtherEnd) const {
    u32 length = mEnd - mBegin;
    u32 rest = (rOther.mEnd - rOther.mBegin) - offset;

    if (rest > length) {
        s32 result =
            sead::SafeString(mBegin).comparen(sead::SafeString(rOther.mBegin + offset), length);
        if (result != 0) {
            return result;
        }

        const Clause* next = mNext;

        if (next == pEnd || !next->isWord()) {
            return -1;
        }

        return next->compareImpl(rOther, offset + length, pEnd, pOtherEnd);
    }

    if (rest < length) {
        s32 result =
            sead::SafeString(mBegin).comparen(sead::SafeString(rOther.mBegin + offset), rest);
        if (result != 0) {
            return result;
        }

        const Clause* otherNext = rOther.mNext;

        if (otherNext == pOtherEnd || !otherNext->isWord()) {
            return 1;
        }

        return -otherNext->compareImpl(*this, rest, pEnd, pOtherEnd);
    }

    s32 result = sead::SafeString(mBegin).comparen(sead::SafeString(rOther.mBegin + offset), rest);

    if (result != 0) {
        return result;
    }

    const Clause* next = mNext;
    const Clause* otherNext = rOther.mNext;

    if (otherNext == pOtherEnd) {
        if (next == pEnd) {
            return 0;
        }

        return next->isWord();
    }

    if (next == pEnd) {
        return -static_cast<s32>(otherNext->isWord());
    }

    s32 state = 0;

    if (next->isWord()) {
        state |= 1;
    }

    if (otherNext->isWord()) {
        state |= 2;
    }

    switch (state) {
    case 3:
        return next->compareImpl(*otherNext, 0, pEnd, pOtherEnd);
    case 2:
        return -1;
    case 1:
        return 1;
    default:
        return 0;
    }
}

/**
 * Compares the clause sequence with a string, joining adjacent word clauses.
 * @param rStr string to compare against
 * @param length length of rStr
 * @param pEnd end of the sequence
 * @return comparison result
 */
s32 Clause::compareImpl(const sead::SafeString& rStr, u32 length, const Clause* pEnd) const {
    s32 clauseLength = mEnd - mBegin;

    if (static_cast<u32>(clauseLength) > length) {
        return -1;
    }

    s32 result = sead::SafeString(mBegin).comparen(rStr, clauseLength);

    if (result != 0) {
        return result;
    }

    u32 rest = length - clauseLength;
    Clause* next = mNext;

    if (next != this && next->isWord()) {
        if (next == pEnd) {
            return -1;
        }

        return next->compareImpl(sead::SafeString(rStr.cstr() + (mEnd - mBegin)), rest, next->getPrev());
    }

    if (rest == 0) {
        return 0;
    }

    char c = rStr.cstr()[mEnd - mBegin];
    return ('A' <= (c & 0xdf) && (c & 0xdf) <= 'Z') || c == '_' || ('0' <= c && c <= '9');
}

/**
 * Compares the clause sequence with a string.
 * @param rStr string to compare against
 * @param length length of rStr
 * @return comparison result
 */
s32 Clause::compare(const sead::SafeString& rStr, u32 length) const {
    return compareImpl(rStr, length, mPrev);
}

/**
 * Compares the clause sequence with another clause sequence.
 * @param rOther clause to compare against
 * @param offset number of characters of rOther already consumed
 * @return comparison result
 */
s32 Clause::compare(const Clause& rOther, u32 offset) const {
    return compareImpl(rOther, offset, mPrev, rOther.mPrev);
}

/**
 * Calculates the CRC32 of a buffer.
 * @param pData data to hash
 * @param size size of the data
 * @param seed initial hash value
 * @return hash
 */
u32 Clause::calcHash(const void* pData, u32 size, u32 seed) {
    u32 hash = ~seed;
    const u8* p = static_cast<const u8*>(pData);

    while (size-- != 0) {
        hash = cHashTable[(hash ^ *p++) & 0xff] ^ (hash >> 8);
    }

    return ~hash;
}

/**
 * Calculates the CRC32 of the clause text.
 * @param seed initial hash value
 * @return hash
 */
u32 Clause::calcHash(u32 seed) const {
    return calcHash(mBegin, mEnd - mBegin, seed);
}

}  // namespace shtxt
}  // namespace agl
