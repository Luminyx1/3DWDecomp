#include "shader_text/aglShaderTextPreprocessor.h"

#include <basis/seadNew.h>
#include <container/seadPtrArray.h>
#include <heap/seadExpHeap.h>
#include <heap/seadFrameHeap.h>
#include <heap/seadUnitHeap.h>
#include <prim/seadDelegate.h>

#include "shader_text/aglShaderTextClause.h"
#include "shader_text/aglShaderTextDefineLinker.h"
#include "shader_text/aglShaderTextLexer.h"
#include "shader_text/aglShaderTextResolver.h"

namespace agl::shtxt {

namespace {

const char* const cSpaceText = "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ";
const char* const cTabText = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
const char* const cLineFeedText = "\r\n\r\n\r\n";

}  // namespace

/**
 * Constructs a preprocessor.
 * @param pHeap parent heap of the work heaps created while preprocessing
 * @param pWorkHeap heap for the clause and macro lists
 */
Preprocessor::Preprocessor(sead::Heap* pHeap, sead::Heap* pWorkHeap)
    : mHeap(pHeap), mWorkHeap(pWorkHeap), mRoot(new (pWorkHeap) Clause()),
      mDefineRoot(new (mWorkHeap) DefineLinker()) {
    Clause::TableChecker checker;
}

/**
 * Destroys the preprocessor.
 */
Preprocessor::~Preprocessor() {
    finalize();
    delete mRoot;
    delete mDefineRoot;
}

/**
 * Releases the clauses, macros and work heaps.
 */
void Preprocessor::finalize() {
    mText = nullptr;
    mRoot->erase();
    mDefineRoot->erase();

    if (mLexicalHeap != nullptr) {
        mLexicalHeap->destroy();
        mLexicalHeap = nullptr;
    }

    if (mSyntacticHeap != nullptr) {
        mSyntacticHeap->destroy();
        mSyntacticHeap = nullptr;
    }

    if (mFormatHeap != nullptr) {
        mFormatHeap->destroy();
        mFormatHeap = nullptr;
    }

    if (mReplaceInfos != nullptr) {
        for (u32 i = 0; i < mReplaceInfoNum; i++) {
            mReplaceInfos[i].mLinker.clear();
        }

        delete[] mReplaceInfos;
        mReplaceInfos = nullptr;
        mReplaceInfoNum = 0;
    }

    if (mDeployInfos != nullptr) {
        delete[] mDeployInfos;
        mDeployInfos = nullptr;
        mDeployInfoNum = 0;
    }

    if (mAppendedMacros != nullptr) {
        DefineLinker* root = mAppendedMacros;

        for (DefineLinker* linker = root->mNext; linker != root;) {
            DefineLinker* next = linker->mNext;
            linker->erase();
            delete linker;
            linker = next;
        }

        mAppendedMacros = nullptr;
    }
}

/**
 * Releases the previous state and sets the text to preprocess.
 * @param pText shader source text
 */
void Preprocessor::initialize(const char* pText) {
    finalize();
    mText = pText;
}

/**
 * Removes a clause from its list and deletes it unless it belongs to the lexical heap.
 * @param pClause clause to remove
 */
void Preprocessor::removeClause(Clause* pClause) const {
    pClause->erase();

    if (!mLexicalHeap->isInclude(pClause)) {
        delete pClause;
    }
}

/**
 * Removes a range of clauses.
 * @param pBegin first clause to remove
 * @param pEnd clause at which removal stops
 * @param includeEnd whether pEnd itself is removed too
 */
void Preprocessor::removeClause(Clause* pBegin, Clause* pEnd, bool includeEnd) const {
    for (Clause* clause = pBegin; clause != pEnd;) {
        Clause* next = clause->mNext;
        removeClause(clause);
        clause = next;
    }

    if (includeEnd) {
        removeClause(pEnd);
    }
}

/**
 * Removes every clause.
 */
void Preprocessor::removeClauseAll() {
    Clause* root = mRoot;
    Clause* clause = root->mNext;

    if (clause == root) {
        return;
    }

    Clause* next = clause->mNext;

    while (true) {
        removeClause(clause);

        if (next == root) {
            break;
        }

        clause = next;
        next = clause->mNext;
    }
}

/**
 * Deletes every macro definition.
 */
void Preprocessor::removeDefineLinkerAll() {
    DefineLinker* root = mDefineRoot;

    for (DefineLinker* linker = root->mNext; linker != root;) {
        DefineLinker* next = linker->mNext;
        linker->erase();
        delete linker;
        linker = next;
    }
}

/**
 * Sets macros that are replaced with the given values.
 * @param pNames macro names
 * @param pValues macro values
 * @param num number of macros
 */
void Preprocessor::setReplacedMacro(const char** pNames, const char** pValues, u32 num) {
    sead::Heap* heap = mWorkHeap;
    Lexer lexer;
    mReplaceInfoNum = num;
    mReplaceInfos = new (heap) MacroReplaceInfo[num];

    for (u32 i = 0; i < mReplaceInfoNum; i++) {
        sead::SafeString name = pNames[i];
        sead::SafeString value = pValues[i];

        Clause* root = new (heap) Clause();
        MacroReplaceInfo& info = mReplaceInfos[i];
        info.mIsValid = false;

        Clause* nameClause = new (heap) Clause();
        nameClause->set(Clause::cType_Word, name.cstr(), name.cstr() + name.calcLength());
        root->mPrev->insertListAfter(nameClause);

        Clause* space = new (heap) Clause();
        space->set(Clause::cType_Space, cSpaceText, cSpaceText + 1);
        root->mPrev->insertListAfter(space);

        lexer.initialize(heap, value.cstr(), root);

        if (lexer.execute(true)) {
            Clause* end = root->mPrev;
            end->erase();
            delete end;
            info.mIsValid = info.mLinker.setDirect(root, 1, true);

            if (!info.mIsValid) {
                info.mLinker.clear();
            }
        } else {
            removeClause(root->mNext, root, true);
        }
    }
}

/**
 * Sets macros whose definitions are kept in the output.
 * @param pNames macro names
 * @param num number of macros
 */
void Preprocessor::setDeployMacro(const char** pNames, u32 num) {
    mDeployInfoNum = num;
    mDeployInfos = new (mWorkHeap) MacroDeployInfo[num];

    for (u32 i = 0; i < mDeployInfoNum; i++) {
        MacroDeployInfo& info = mDeployInfos[i];
        info.mName = pNames[i];
        info.mLength = info.mName.calcLength();
    }
}

/**
 * Appends copies of a list of macro definitions that are defined before preprocessing.
 * @param pLinker list of definitions
 */
void Preprocessor::appendMacro(const DefineLinker* pLinker) {
    sead::Heap* heap = mWorkHeap;

    if (mAppendedMacros == nullptr) {
        mAppendedMacros = new (heap) DefineLinker();
    }

    if (DefineLinker* list = pLinker->cloneAll(heap, heap)) {
        mAppendedMacros->insertListAfter(list);
    }
}

/**
 * Tokenizes the text and resolves macros and preprocessor directives.
 * @param flags combination of Flag values
 * @param clauseNum expected number of clauses (0 for the default)
 * @param appendHeapSize size of the heap for macro copies (0 for the default)
 * @return whether preprocessing succeeded
 */
bool Preprocessor::preprocess(u32 flags, u64 clauseNum, u64 appendHeapSize) {
    mLexicalHeap = sead::FrameHeap::create(0, "lexical", mHeap, 8, sead::Heap::cHeapDirection_Forward,
                                           false);
    bool isSucceeded;
    {
        Lexer lexer;
        lexer.initialize(mLexicalHeap, mText, mRoot);
        isSucceeded = lexer.execute(false);
        mErrorMessage = lexer.getErrorMessage();
    }

    mLexicalHeap->adjust();

    if (!isSucceeded) {
        return false;
    }

    mSyntacticHeap = sead::ExpHeap::create(0, "syntactic", mHeap, 8,
                                           sead::Heap::cHeapDirection_Forward, false);
    if (appendHeapSize == 0) {
        appendHeapSize = 0x40000;
    }

    sead::UnitHeap* clauseHeap = sead::UnitHeap::create(
        clauseNum == 0 ? 0xa00000 : clauseNum * sizeof(Clause), "clause", sizeof(Clause), 4,
        mSyntacticHeap, false);
    sead::FrameHeap* appendHeap = sead::FrameHeap::create(
        appendHeapSize, "append", mSyntacticHeap, 8, sead::Heap::cHeapDirection_Forward, false);
    sead::AnyDelegate1Const<Clause*> removeDelegate(
        sead::Delegate1<const Preprocessor, Clause*>(this, &Preprocessor::removeClause));
    {
        Resolver resolver;
        appendHeap->mFlag.setBit(sead::Heap::Flag::cDisposing);

        if (mAppendedMacros != nullptr) {
            DefineLinker* root = mDefineRoot;
            root->insertListAfter(mAppendedMacros->cloneAll(appendHeap, clauseHeap));
        }

        resolver.initialize(mSyntacticHeap, clauseHeap, appendHeap, mText, mRoot, mDefineRoot,
                            &removeDelegate);
        resolver.setMacroReplaceInfo(mReplaceInfos, mReplaceInfoNum);
        resolver.setMacroDeployInfo(mDeployInfos, mDeployInfoNum);
        isSucceeded = resolver.execute(flags & cFlag_ResolveMacro, flags & cFlag_ResolveExpression,
                                       flags & cFlag_ResolveProgram, true,
                                       flags & cFlag_ResolveDefine, flags & cFlag_ResolveBranch,
                                       flags & cFlag_ResolveTernary);
    }

    mSyntacticHeap->adjust();

    if (!isSucceeded) {
        return false;
    }

    if (flags & (cFlag_RemoveComment | cFlag_ForceLF | cFlag_ReduceSpace | cFlag_ReduceLF |
                 cFlag_Format)) {
        mFormatHeap = sead::ExpHeap::create(0, "format", mHeap, 8,
                                            sead::Heap::cHeapDirection_Forward, false);
        if (flags & cFlag_RemoveComment) {
            removeComment();
        }

        if (flags & cFlag_ForceLF) {
            forceLF();
        }

        if (flags & cFlag_ReduceSpace) {
            reduceSpace();
        }

        if (flags & cFlag_ReduceLF) {
            reduceLF();
        }

        if (flags & cFlag_Format) {
            format(flags & cFlag_FormatWithTab);
        }

        mFormatHeap->adjust();
    }

    return true;
}

/**
 * Removes every comment clause.
 */
void Preprocessor::removeComment() {
    Clause* root = mRoot;

    for (Clause* clause = root->mNext; clause != root;) {
        Clause* next = clause->mNext;

        if (clause->isComment()) {
            removeClause(clause);
        }

        clause = next;
    }
}

namespace {

Clause* skipSpace(Clause* pClause) {
    while (pClause->mType == Clause::cType_Space) {
        pClause = pClause->mNext;
    }

    return pClause;
}

bool needsLineFeedAfter(const Clause* pClause) {
    return !pClause->isComment() && pClause->mType != Clause::cType_LineFeed &&
           pClause->mType != Clause::cType_BackSlash;
}

}  // namespace

/**
 * Puts braces and statements on their own lines.
 */
void Preprocessor::forceLF() {
    Clause* root = mRoot;

    for (Clause* clause = root->mNext; clause != root;) {
        switch (clause->mType) {
        case Clause::cType_LBrace: {
            Clause* next = skipSpace(clause->mNext);

            if (needsLineFeedAfter(next)) {
                clause->mPrev->insertAfter(new (mFormatHeap) Clause(
                    Clause::cType_LineFeed, cLineFeedText, cLineFeedText + 2));
                Clause* after = clause->mNext;
                clause->insertAfter(new (mFormatHeap) Clause(Clause::cType_LineFeed,
                                                             cLineFeedText, cLineFeedText + 2));
                clause = after;
            } else {
                clause = next;
            }

            break;
        }
        case Clause::cType_RBrace: {
            Clause* next = skipSpace(clause->mNext);

            if (needsLineFeedAfter(next)) {
                clause->mPrev->insertAfter(new (mFormatHeap) Clause(
                    Clause::cType_LineFeed, cLineFeedText, cLineFeedText + 2));
                Clause* after = clause->mNext;

                if (next->mType != Clause::cType_Semicolon) {
                    clause->insertAfter(new (mFormatHeap) Clause(
                        Clause::cType_LineFeed, cLineFeedText, cLineFeedText + 2));
                }

                clause = after;
            } else {
                clause = next;
            }

            break;
        }
        case Clause::cType_Semicolon: {
            Clause* next = skipSpace(clause->mNext);

            if (needsLineFeedAfter(next)) {
                Clause* after = clause->mNext;
                clause->insertAfter(new (mFormatHeap) Clause(Clause::cType_LineFeed,
                                                             cLineFeedText, cLineFeedText + 2));
                clause = after;
            } else {
                clause = next;
            }

            break;
        }
        case Clause::cType_Sharp:
            do {
                while (clause->mType != Clause::cType_LineFeed) {
                    clause = clause->mNext;
                }

                if (clause->mPrev->mType != Clause::cType_BackSlash) {
                    break;
                }

                clause = clause->mNext;
            } while (clause->mType != Clause::cType_End);
            break;
        default:
            clause = clause->mNext;
            break;
        }
    }
}

/**
 * Replaces runs of white space with a single space.
 */
void Preprocessor::reduceSpace() {
    Clause* root = mRoot;

    for (Clause* clause = root->mNext; clause != root;) {
        if (clause->mType != Clause::cType_Space) {
            clause = clause->mNext;
            continue;
        }

        if (clause->mEnd - clause->mBegin >= 5) {
            clause->mPrev->insertAfter(new (mFormatHeap) Clause(Clause::cType_Space, cSpaceText,
                                                                cSpaceText + 1));
        } else {
            clause = clause->mNext;
        }

        if (clause->mType == Clause::cType_Space) {
            Clause* end = clause;

            do {
                end = end->mNext;
            } while (end->mType == Clause::cType_Space);

            if (end != clause) {
                removeClause(clause, end, false);
            }

            clause = end;
        }
    }
}

/**
 * Replaces runs of line feeds with a single line feed.
 */
void Preprocessor::reduceLF() {
    Clause* root = mRoot;

    for (Clause* clause = root->mNext; clause != root;) {
        if (clause->mType != Clause::cType_LineFeed) {
            clause = clause->mNext;
            continue;
        }

        if (clause->mEnd - clause->mBegin >= 5) {
            clause->mPrev->insertAfter(new (mFormatHeap) Clause(
                Clause::cType_LineFeed, cLineFeedText, cLineFeedText + 2));
        } else {
            clause = clause->mNext;
        }

        if (clause->mType == Clause::cType_Space || clause->mType == Clause::cType_LineFeed) {
            Clause* end = clause;

            do {
                end = end->mNext;
            } while (end->mType == Clause::cType_Space || end->mType == Clause::cType_LineFeed);

            if (end != clause) {
                removeClause(clause, end, false);
            }

            clause = end;
        }
    }
}

namespace {

s32 calcColumn(const Clause* pBegin, const Clause* pEnd, s32 column) {
    for (const Clause* clause = pBegin; clause != pEnd; clause = clause->mNext) {
        for (const char* p = clause->mBegin; p < clause->mEnd; p++) {
            column += *p == '\t' ? 4 - (column & 3) : 1;
        }
    }

    return column;
}

}  // namespace

/**
 * Indents every line according to the bracket nesting.
 * @param useTab whether indentation uses tabs
 */
void Preprocessor::format(bool useTab) {
    sead::PtrArray<Clause> stack;
    stack.allocBuffer(0x100, mFormatHeap, -4);

    Clause* root = mRoot;
    Clause* bracket = nullptr;
    s32 indent = -1;

    for (Clause* clause = root->mNext; clause != root; clause = clause->mNext) {
        if (clause->getInfo().mIsOpenBracket) {
            stack.pushBack(clause);
            bracket = clause;
            indent = -1;
            continue;
        }

        if (clause->getInfo().mIsCloseBracket) {
            stack.popBack();
            bracket = stack.size() > 0 ? stack.back() : nullptr;
            indent = -1;
            continue;
        }

        if (clause->mType == Clause::cType_Sharp) {
            Clause* prev = clause->mPrev;

            while (prev->mType == Clause::cType_Space) {
                prev = prev->mPrev;
            }

            removeClause(prev->mNext, clause, false);

            while (clause->mType != Clause::cType_LineFeed) {
                clause = clause->mNext;
            }
        }

        if (clause->mType != Clause::cType_LineFeed) {
            continue;
        }

        Clause* prev = clause->mPrev;

        while (prev->mType == Clause::cType_Space) {
            prev = prev->mPrev;
        }

        removeClause(prev->mNext, clause, false);
        Clause* next = clause->mNext;

        if (next->mType == Clause::cType_Space) {
            Clause* end = next;

            while (end->mType == Clause::cType_Space) {
                end = end->mNext;
            }

            removeClause(next, end, false);
        }

        if (bracket == nullptr) {
            continue;
        }

        if (indent < 0) {
            Clause* lineHead = bracket;

            while ((lineHead->mType | 2) != 2) {
                lineHead = lineHead->mPrev;
            }

            s32 column = calcColumn(lineHead->mNext, bracket, 0);
            Clause* after = bracket->mNext;

            while (after->mType == Clause::cType_Space || after->isComment()) {
                after = after->mNext;
            }

            if (after->mType == Clause::cType_LineFeed || after->mType == Clause::cType_BackSlash) {
                indent = column + 4 - (column & 3);
            } else {
                indent = calcColumn(after, after->mNext, column + 1);
            }
        }

        s32 tabNum = indent / 4;
        s32 spaceNum = useTab ? indent % 4 : indent;

        if (useTab && tabNum != 0) {
            clause->insertAfter(new (mFormatHeap) Clause(Clause::cType_Space, cTabText,
                                                         cTabText + tabNum));
            clause = clause->mNext;
        }

        if (spaceNum != 0) {
            clause->insertAfter(new (mFormatHeap) Clause(Clause::cType_Space, cSpaceText,
                                                         cSpaceText + spaceNum));
            clause = clause->mNext;
        }
    }

    stack.freeBuffer();

    u32 lineFeedNum = 0;

    for (Clause* clause = root->mNext; clause != root;) {
        Clause* next = clause->mNext;
        u32 num = 0;

        if (clause->mType == Clause::cType_LineFeed) {
            num = clause->calcLineFeedCount() + lineFeedNum;

            if (num >= 3) {
                if (lineFeedNum == 2) {
                    removeClause(clause);
                } else {
                    clause->set(Clause::cType_LineFeed, cLineFeedText,
                                cLineFeedText + (2 - lineFeedNum) * 2);
                }

                num = 2;
            }
        }

        lineFeedNum = num;
        clause = next;
    }
}

/**
 * Writes the text of every clause into a string.
 * @param pDst destination string
 * @return length of the text plus one
 */
s32 Preprocessor::construct(sead::BufferedSafeString* pDst) const {
    pDst->clear();
    s32 length = 0;
    Clause* root = mRoot;

    for (Clause* clause = root->mNext; clause != root; clause = clause->mNext) {
        length += clause->appendTo(pDst, length);
    }

    return length + 1;
}

/**
 * Calculates the length of the text of every clause.
 * @return total length
 */
u64 Preprocessor::calcConstructLength() const {
    u64 length = 0;

    for (Clause* clause = mRoot->mNext; clause != mRoot; clause = clause->mNext) {
        length += clause->mEnd - clause->mBegin;
    }

    return length;
}

}  // namespace agl::shtxt
