#include "shader_text/aglShaderTextLexer.h"

#include <basis/seadNew.h>

#include "detail/aglShaderTextUtil.h"

namespace agl::shtxt {

/**
 * Constructs an empty lexer.
 */
Lexer::Lexer() = default;

/**
 * Destroys the lexer.
 */
Lexer::~Lexer() {}

/**
 * Sets the heap, text and root clause used by execute.
 * @param pHeap heap for new clauses
 * @param pText text to tokenize
 * @param pRoot list the clauses are appended to
 */
void Lexer::initialize(sead::Heap* pHeap, const char* pText, Clause* pRoot) {
    mHeap = pHeap;
    mText = pText;
    mRoot = pRoot;
}

/**
 * Sets the current clause range to start at the cursor.
 * @param length length of the range
 * @return end of the range
 */
const char* Lexer::setupCurrentRange(u64 length) {
    mBegin = mCurrent;
    mEnd = mCurrent + length;
    return mEnd;
}

/**
 * Checks for a numeric literal at the cursor and sets the current range to it.
 * @return type of the literal, or cType_None
 */
Clause::Type Lexer::findNumberBlock() const {
    Clause::Type type = Clause::cType_None;
    const char* end = Clause::findNumberBlock(&type, mCurrent);
    if (type != Clause::cType_None) {
        mBegin = mCurrent;
        mEnd = end;
    }

    return type;
}

/**
 * Creates a clause for the current range and appends it to the root list.
 * @param type clause type
 * @return new clause
 */
Clause* Lexer::createClause(u32 type) const {
    Clause* clause = new (mHeap) Clause(static_cast<Clause::Type>(type), mBegin, mEnd);
    mRoot->pushBack(clause);
    return clause;
}

/**
 * Splits the text into clauses.
 * @param skipSpace whether white space and line feeds are skipped
 * @return whether the text was tokenized without errors
 */
bool Lexer::execute(bool skipSpace) {
    mCurrent = mText;
    mErrorMessage = "";

    bool isContinue;
    do {
        if (skipSpace) {
            const char* p = mCurrent;
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
                p++;
            }

            mCurrent = p;
        }

        isContinue = true;
        u32 type = Clause::cType_End;
        switch (*mCurrent) {
        case '\t':
        case ' ': {
            const char* p = mCurrent;
            mBegin = p;
            while (*p == ' ' || *p == '\t') {
                p++;
            }

            mEnd = p;
            mCurrent = p;
            type = Clause::cType_Space;
            break;
        }
        case '\n':
        case '\r': {
            const char* p = mCurrent;
            mBegin = p;
            while (*p == '\r' || *p == '\n') {
                p++;
            }

            mEnd = p;
            mCurrent = p;
            type = Clause::cType_LineFeed;
            break;
        }
        case '!':
            if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_NotEqual;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Not;
            }

            break;
        case '"':
            mCurrent = setupCurrentRange(1);
            createClause(Clause::cType_DoubleQuote);
            mBegin = mCurrent;
            for (mEnd = mCurrent; *mEnd != '"'; mEnd++) {
            }

            mCurrent = mEnd;
            createClause(Clause::cType_Word);
            mCurrent = setupCurrentRange(1);
            createClause(Clause::cType_DoubleQuote);
            continue;
        case '#':
            if (mCurrent[1] == '#') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_DoubleSharp;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Sharp;
            }

            break;
        case '$':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_Dollar;
            break;
        case '%':
            if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_ModAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Mod;
            }

            break;
        case '&':
            if (mCurrent[1] == '&') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_LogicalAnd;
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_AndAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_And;
            }

            break;
        case '\'':
            mCurrent = setupCurrentRange(1);
            createClause(Clause::cType_SingleQuote);
            mBegin = mCurrent;
            for (mEnd = mCurrent; *mEnd != '\''; mEnd++) {
            }

            mCurrent = mEnd;
            createClause(Clause::cType_Word);
            mCurrent = setupCurrentRange(1);
            createClause(Clause::cType_SingleQuote);
            continue;
        case '(':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_LParen;
            break;
        case ')':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_RParen;
            break;
        case '*':
            if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_MulAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Mul;
            }

            break;
        case '+':
            if (mCurrent[1] == '+') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_Increment;
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_PlusAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Plus;
            }

            break;
        case ',':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_Comma;
            break;
        case '-':
            if (mCurrent[1] == '-') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_Decrement;
            } else if (mCurrent[1] == '>') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_Arrow;
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_MinusAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Minus;
            }

            break;
        case '.':
            type = findNumberBlock();
            if (type != Clause::cType_None) {
                mCurrent = mEnd;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Dot;
            }

            break;
        case '/':
            if (mCurrent[1] == '*') {
                mBegin = mCurrent;
                bool hasLineFeed = false;
                for (mEnd = mCurrent + 2; *mEnd != '\0'; mEnd++) {
                    hasLineFeed |= *mEnd == '\r' || *mEnd == '\n';
                    if (*mEnd == '*' && mEnd[1] == '/') {
                        break;
                    }
                }

                if (*mEnd != '\0') {
                    mEnd += 2;
                    mCurrent = mEnd;
                    type = hasLineFeed ? Clause::cType_BlockCommentLF : Clause::cType_BlockComment;
                    break;
                }

                mErrorMessage = "invalid";
                isContinue = false;
                goto word;
            }

            if (mCurrent[1] == '/') {
                mBegin = mCurrent;
                for (mEnd = mCurrent; !(*mEnd == '\0' || *mEnd == '\n' || *mEnd == '\r');
                     mEnd++) {
                }

                mCurrent = mEnd;
                type = Clause::cType_LineComment;
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_DivAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Div;
            }

            break;
        case ':':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_Colon;
            break;
        case ';':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_Semicolon;
            break;
        case '<':
            if (mCurrent[1] == '<') {
                if (mCurrent[2] == '=') {
                    mCurrent = setupCurrentRange(3);
                    type = Clause::cType_ShiftLAssign;
                } else {
                    mCurrent = setupCurrentRange(2);
                    type = Clause::cType_ShiftL;
                }
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_LessEqual;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Less;
            }

            break;
        case '=':
            if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_Equal;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Assign;
            }

            break;
        case '>':
            if (mCurrent[1] == '>') {
                if (mCurrent[2] == '=') {
                    mCurrent = setupCurrentRange(3);
                    type = Clause::cType_ShiftRAssign;
                } else {
                    mCurrent = setupCurrentRange(2);
                    type = Clause::cType_ShiftR;
                }
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_GreaterEqual;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Greater;
            }

            break;
        case '?':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_Question;
            break;
        case '@':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_At;
            break;
        case '[':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_LBracket;
            break;
        case '\\':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_BackSlash;
            break;
        case ']':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_RBracket;
            break;
        case '^':
            if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_XorAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Xor;
            }

            break;
        case '`':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_BackQuote;
            break;
        case '{':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_LBrace;
            break;
        case '|':
            if (mCurrent[1] == '|') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_LogicalOr;
            } else if (mCurrent[1] == '=') {
                mCurrent = setupCurrentRange(2);
                type = Clause::cType_OrAssign;
            } else {
                mCurrent = setupCurrentRange(1);
                type = Clause::cType_Or;
            }

            break;
        case '}':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_RBrace;
            break;
        case '~':
            mCurrent = setupCurrentRange(1);
            type = Clause::cType_Tilde;
            break;
        case '\0':
            mCurrent = setupCurrentRange(1);
            isContinue = false;
            break;
        default:
        word:
            type = findNumberBlock();
            if (type != Clause::cType_None) {
                mCurrent = mEnd;
            } else {
                mBegin = mCurrent;
                for (mEnd = mCurrent; !detail::IsDelimiter(*mEnd); mEnd++) {
                }

                mCurrent = mEnd;
                type = Clause::cType_Word;
            }

            break;
        }

        createClause(type);
    } while (isContinue);

    return mErrorMessage.isEmpty();
}

}  // namespace agl::shtxt
