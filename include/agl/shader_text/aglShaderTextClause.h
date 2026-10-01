#pragma once

#include <prim/seadSafeString.h>

namespace sead {
class Heap;
}

namespace agl::shtxt {

namespace helper {

template <typename T>
class Knot {
public:
    class constIterator {
    public:
        constIterator() = default;
        explicit constIterator(const T* pNode) : mNode(pNode) {}

        const T& operator*() const { return *mNode; }
        const T* operator->() const { return mNode; }
        constIterator& operator++() {
            mNode = mNode->mNext;
            return *this;
        }
        bool operator==(const constIterator& rOther) const { return mNode == rOther.mNode; }
        bool operator!=(const constIterator& rOther) const { return mNode != rOther.mNode; }

        const T* mNode = nullptr;
    };

    Knot() : mPrev(static_cast<T*>(this)), mNext(static_cast<T*>(this)) {}

    T* getPrev() const { return mPrev; }
    T* getNext() const { return mNext; }
    bool isAlone() const { return mNext == this; }

    void erase() {
        mPrev->mNext = mNext;
        mNext->mPrev = mPrev;
        mPrev = static_cast<T*>(this);
        mNext = static_cast<T*>(this);
    }

    void insertBefore(T* pNode) {
        T* prev = mPrev;
        prev->mNext = pNode;
        pNode->mPrev = prev;
        pNode->mNext = static_cast<T*>(this);
        mPrev = pNode;
    }

    void insertListBefore(T* pList) {
        T* prev = mPrev;
        T* last = pList->mPrev;
        prev->mNext = pList;
        pList->mPrev = prev;
        last->mNext = static_cast<T*>(this);
        mPrev = last;
    }

    void insertAfter(T* pNode) {
        if (!pNode->isAlone()) {
            pNode->erase();
        }
        mNext->insertBefore(pNode);
    }

    void insertListAfter(T* pList) { mNext->insertListBefore(pList); }

    void pushBack(T* pNode) { mPrev->insertAfter(pNode); }

    T* mPrev;
    T* mNext;
};

}  // namespace helper

class Clause : public helper::Knot<Clause> {
public:
    class TableChecker {
    public:
        TableChecker();
    };

    enum Type {
        cType_None = 0,
        cType_Space = 1,
        cType_LineFeed = 2,
        cType_LineComment = 3,
        cType_BlockComment = 4,
        cType_BlockCommentLF = 5,
        cType_Int = 6,
        cType_Hex = 7,
        cType_Oct = 8,
        cType_Real = 9,
        cType_Word = 10,
        cType_LParen = 11,
        cType_RParen = 12,
        cType_LBracket = 13,
        cType_RBracket = 14,
        cType_LBrace = 15,
        cType_RBrace = 16,
        cType_Dot = 17,
        cType_Comma = 18,
        cType_Colon = 19,
        cType_Semicolon = 20,
        cType_SingleQuote = 21,
        cType_DoubleQuote = 22,
        cType_Sharp = 23,
        cType_BackSlash = 24,
        cType_Arrow = 25,
        cType_Plus = 26,
        cType_Minus = 27,
        cType_Mul = 28,
        cType_Div = 29,
        cType_Mod = 30,
        cType_Assign = 31,
        cType_Tilde = 32,
        cType_And = 33,
        cType_Or = 34,
        cType_Xor = 35,
        cType_ShiftL = 36,
        cType_ShiftR = 37,
        cType_Less = 38,
        cType_Greater = 39,
        cType_LogicalAnd = 40,
        cType_LogicalOr = 41,
        cType_LessEqual = 42,
        cType_GreaterEqual = 43,
        cType_NotEqual = 44,
        cType_Equal = 45,
        cType_PlusAssign = 46,
        cType_MinusAssign = 47,
        cType_MulAssign = 48,
        cType_DivAssign = 49,
        cType_ModAssign = 50,
        cType_AndAssign = 51,
        cType_OrAssign = 52,
        cType_XorAssign = 53,
        cType_ShiftLAssign = 54,
        cType_ShiftRAssign = 55,
        cType_Increment = 56,
        cType_Decrement = 57,
        cType_Not = 58,
        cType_Question = 59,
        cType_DoubleSharp = 60,
        cType_BackQuote = 61,
        cType_Dollar = 62,
        cType_At = 63,
        cType_End = 64,
    };

    Clause();
    Clause(Type type, const char* pBegin, const char* pEnd);
    ~Clause();

    static Clause* clone(sead::Heap* pHeap, const constIterator& rBegin, const constIterator& rEnd,
                         bool includeEnd);
    static const char* findNumberBlock(Type* pType, const char* pText);

    void set(Type type, const char* pBegin, const char* pEnd);
    s32 appendTo(sead::BufferedSafeString* pDst) const;
    s32 appendTo(sead::BufferedSafeString* pDst, u32 at) const;
    s32 copyTo(sead::BufferedSafeString* pDst) const;
    f64 toNumber() const;
    f64 forceNumber() const;
    s32 calcLineFeedCount() const;
    void chop(u32 num);
    s32 compareImpl(const Clause& rOther, u32 offset, const Clause* pEnd,
                    const Clause* pOtherEnd) const;
    s32 compareImpl(const sead::SafeString& rStr, u32 length, const Clause* pEnd) const;
    s32 compare(const sead::SafeString& rStr, u32 length) const;
    s32 compare(const Clause& rOther, u32 offset) const;
    static u32 calcHash(const void* pData, u32 size, u32 seed);
    u32 calcHash(u32 seed) const;

    struct CharacterInfo {
        u32 mType;
        bool mIsSpace;
        bool mIsComment;
        bool mIsNumber;
        bool mIsOperator;
        bool mIsUnaryOperator;
        bool mIsBinaryOperator;
        bool mIsArithmeticOperator;
        bool mIsBitOperator;
        bool mIsLogicalOperator;
        bool mIsAssignOperator;
        bool mIsSpecialOperator;
        bool mIsOpenBracket;
        bool mIsCloseBracket;
        bool mIsSeparator;
        bool mIsInvalid;
        bool _13;
    };
    static_assert(sizeof(CharacterInfo) == 0x14);

    static const CharacterInfo cCharacterTable[cType_End + 1];

    Type getType() const { return static_cast<Type>(mType); }
    const CharacterInfo& getInfo() const { return cCharacterTable[mType]; }
    bool isSpace() const { return getInfo().mIsSpace; }
    bool isComment() const { return getInfo().mIsComment; }
    bool isNumber() const { return getInfo().mIsNumber; }
    bool isWord() const { return isNumber() || mType == cType_Word; }
    bool isOperator() const { return getInfo().mIsOperator; }
    bool isOpenBracket() const { return getInfo().mIsOpenBracket; }
    bool isCloseBracket() const { return getInfo().mIsCloseBracket; }
    bool isSeparator() const { return getInfo().mIsSeparator; }
    u32 getLength() const { return mEnd - mBegin; }
    bool isSpaceOrLineFeed() const { return mType == cType_Space || mType == cType_LineFeed; }

    u8 mType;
    u8 mFlag;
    const char* mBegin;
    const char* mEnd;

    static bool cTableChecked;
    static u32 cHashTable[256];
};
static_assert(sizeof(Clause) == 0x28);

}  // namespace agl::shtxt
