#include "shader_text/aglShaderTextResolver.h"

#include <basis/seadNew.h>
#include <prim/seadSafeString.h>

#include "shader_text/aglShaderTextClause.h"
#include "shader_text/aglShaderTextSyntaxTree.h"

namespace agl::shtxt {

namespace {

struct MacroArgument {
    Clause* mBegin;
    Clause* mEnd;
    Clause* mName;
    u32 mHash;
};

static_assert(sizeof(MacroArgument) == 0x20);

/**
 * Copies a range of clauses into a text buffer for diagnostics.
 * @param pBegin first clause
 * @param pEnd clause after the range
 */
void dumpClauses(const Clause* pBegin, const Clause* pEnd) {
    sead::FixedSafeString<4096> text;

    for (const Clause* c = pBegin; c != pEnd; c = c->mNext) {
        c->copyTo(&text);
    }
}

Clause* skipSpace(Clause* pClause) {
    while (pClause->isSpace()) {
        pClause = pClause->mNext;
    }

    return pClause;
}

/**
 * Finds the directive that ends the current preprocessor block.
 * @param pBegin clause to start searching at
 * @param pEnd clause at which searching stops
 * @param isEndifOnly whether only #endif ends the block (otherwise #elif and #else too)
 * @return the '#' clause of the directive, or nullptr
 */
Clause* findPreprocessEnd(Clause* pBegin, Clause* pEnd, bool isEndifOnly) {
    s32 depth = 0;

    for (Clause* c = pBegin; c != pEnd; c = c->mNext) {
        if (c->mType != Clause::cType_Sharp) {
            continue;
        }

        Clause* directive = skipSpace(c->mNext);

        if (directive->mType != Clause::cType_Word) {
            continue;
        }

        if (directive->compare("if", 2) == 0 || directive->compare("ifdef", 5) == 0 ||
            directive->compare("ifndef", 6) == 0) {
            depth++;
        } else if (directive->compare("endif", 5) == 0) {
            if (depth == 0) {
                return c;
            }

            depth--;
        } else if (!isEndifOnly) {
            if (directive->compare("elif", 4) == 0) {
                if (depth == 0) {
                    return c;
                }
            } else if (directive->compare("else", 4) == 0 && depth == 0) {
                return c;
            }
        }
    }

    return nullptr;
}

Clause* skipElseBlock(Clause* pBegin, Clause* pEnd) {
    Clause* c = pBegin;
    c = skipSpace(c);

    bool isElseFollowing = false;

    for (;;) {
        Clause* word = c;
        word = skipSpace(word);

        if (word->mType != Clause::cType_Word || word->compare("else", 4) != 0) {
            if (!isElseFollowing) {
                return c;
            }
        } else {
            do {
                word = word->mNext;
            } while (word->isSpace());

            if (word->mType == Clause::cType_Word && word->compare("if", 2) == 0) {
                do {
                    word = word->mNext;
                } while (word->isSpace());

                if (word->isOpenBracket()) {
                    s32 depth = 0;

                    while (depth != 1 || !word->isCloseBracket()) {
                        depth += word->isOpenBracket() ? 1 : -word->isCloseBracket();

                        do {
                            word = word->mNext;
                        } while (word->isSpace());
                    }
                }

                do {
                    word = word->mNext;
                } while (word->isSpace());
            }

            if (word->isOpenBracket()) {
                word = skipSpace(word);

                if (word->isOpenBracket()) {
                    s32 depth = 0;

                    while (depth != 1 || !word->isCloseBracket()) {
                        depth += word->isOpenBracket() ? 1 : -word->isCloseBracket();

                        do {
                            word = word->mNext;
                        } while (word->isSpace());
                    }
                }

                c = word->mNext;
                isElseFollowing = false;
                continue;
            }

            c = word;
        }

        u32 type;

        do {
            type = c->mType;
            c = c->mNext;
        } while (type != Clause::cType_Semicolon);

        Clause* next = c;
        next = skipSpace(next);

        if (next->mType != Clause::cType_Word || next->compare("else", 4) != 0) {
            return c;
        }

        isElseFollowing = true;
    }
}

Clause* findDefineEnd(Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        u32 type = c->mType;

        if (type == Clause::cType_End) {
            return c;
        }

        if (type != Clause::cType_BlockCommentLF && type != Clause::cType_LineFeed) {
            continue;
        }

        Clause* prev = c->mPrev;

        while (prev->mType == Clause::cType_Space || prev->mType == Clause::cType_LineFeed) {
            prev = prev->mPrev;
        }

        if (prev->mType != Clause::cType_BackSlash) {
            return c;
        }

        if (type == Clause::cType_LineFeed && c->calcLineFeedCount() == 1) {
            continue;
        }

        if (c->mType != Clause::cType_BlockCommentLF) {
            return c;
        }
    }

    return nullptr;
}

Clause* findTernaryBegin(Clause* pQuestion) {
    Clause* c = pQuestion;

    for (;;) {
        do {
            c = c->mPrev;
        } while (c->isSpace());

        if (c->isCloseBracket()) {
            s32 depth = 0;

            while (depth != 1 || !c->isOpenBracket()) {
                depth += c->isCloseBracket() ? 1 : -c->isOpenBracket();

                do {
                    c = c->mPrev;
                } while (c->isSpace());
            }

            continue;
        }

        u32 type = c->mType;

        if (type == Clause::cType_Colon || type == Clause::cType_Question ||
            c->getInfo().mIsAssignOperator) {
            break;
        }

        if (type == Clause::cType_Dot || c->isOperator() || c->isWord()) {
            continue;
        }

        break;
    }

    return c;
}

}  // namespace

/**
 * Constructs an empty resolver.
 */
Resolver::Resolver() = default;

/**
 * Destroys the resolver.
 */
Resolver::~Resolver() = default;

/**
 * Removes a clause through the remove delegate.
 * @param pClause clause to remove
 */
void Resolver::removeClause(Clause* pClause) const {
    (*mRemoveDelegate)(pClause);
}

/**
 * Removes a range of clauses through the remove delegate.
 * @param pBegin first clause to remove
 * @param pEnd end of the range
 * @param includeEnd whether pEnd is removed as well
 */
void Resolver::removeClause(Clause* pBegin, Clause* pEnd, bool includeEnd) const {
    for (Clause* c = pBegin; c != pEnd;) {
        Clause* next = c->mNext;
        removeClause(c);
        c = next;
    }

    if (includeEnd) {
        removeClause(pEnd);
    }
}

void Resolver::restructDefineStack(DefineStackInfo* pInfo) {
    sead::MemUtil::fillZero(pInfo->mTable, sizeof(pInfo->mTable));

    if (pInfo->mRoot == pInfo->mRoot->mNext) {
        return;
    }

    DefineLinker list;
    list.insertListAfter(pInfo->mRoot);
    pInfo->mRoot->erase();

    for (DefineLinker* linker = list.mNext; linker != &list;) {
        DefineLinker* next = linker->mNext;
        DefineLinker*& bucket = pInfo->mTable[static_cast<u8>(linker->mHash)];
        DefineLinker* at = bucket;

        if (!at) {
            bucket = linker;
            at = pInfo->mRoot->mPrev;
        }

        at->insertAfter(linker);
        linker = next;
    }
}

void Resolver::removeDefineStack(DefineStackInfo* pInfo, u32 depth) {
    DefineLinker* root = pInfo->mRoot;

    for (DefineLinker* linker = root->mNext; linker != root;) {
        DefineLinker* next = linker->mNext;

        if (linker->mDepth == depth) {
            for (u32 i = 0; i < mDeployInfoNum; i++) {
                if (linker->mName->compare(mDeployInfos[i].mName, mDeployInfos[i].mLength) == 0) {
                    mDeployInfos[i].mIsActive = false;
                    break;
                }
            }

            linker->erase();
            delete linker;
        }

        linker = next;
    }
}

/**
 * Enters a new define scope.
 */
void Resolver::pushDefineStack() {
    mDefineStackDepth++;
}

/**
 * Leaves the current define scope, removing the macros defined in it.
 */
void Resolver::popDefineStack() {
    removeDefineStack(&mDefineStack, mDefineStackDepth--);
    restructDefineStack(&mDefineStack);
}

/**
 * Creates a macro definition and registers it in the define stack, replacing older ones.
 * @param pBegin '#' clause of the #define directive
 * @param pEnd end of the directive
 * @param pIsReplaced receives whether a replacement value was applied
 * @return the new definition
 */
DefineLinker* Resolver::createDefineLinker(Clause* pBegin, Clause* pEnd, bool* pIsReplaced) {
    DefineLinker* linker = new (mHeap) DefineLinker();
    *pIsReplaced = false;
    linker->set(mUnitHeap, pBegin, pEnd);
    linker->mDepth = mDefineStackDepth;

    for (u32 i = 0; i < mReplaceInfoNum; i++) {
        if (mReplaceInfos[i].mIsValid && mReplaceInfos[i].mLinker.mHash == linker->mHash &&
            linker->mName->compare(*mReplaceInfos[i].mLinker.mName, 0) == 0) {
            linker->replace(mUnitHeap, mReplaceInfos[i].mLinker.mValueBegin,
                            mReplaceInfos[i].mLinker.mValueEnd, false);
            *pIsReplaced = true;
        }
    }

    u32 hash = linker->mHash;
    u32 index = hash & 0xff;
    DefineLinker*& bucket = mDefineStack.mTable[index];
    DefineLinker* at = bucket;

    if (at) {
        DefineLinker* root = mDefineStack.mRoot;
        const Clause* name = linker->mName;

        for (DefineLinker* other = at; root != other; other = other->mNext) {
            if ((other->mHash & 0xff) != index) {
                break;
            }

            if (other->mHash == hash && *name->mBegin == *other->mName->mBegin &&
                name->compare(*other->mName, 0) == 0) {
                if (other->mIsEnabled) {
                    sead::FixedSafeString<128> name;

                    for (Clause* c = other->mName; c->mType == Clause::cType_Word; c = c->mNext) {
                        c->appendTo(&name);
                    }
                }

                if (bucket == other) {
                    bucket = linker;
                    at = other->mPrev;
                }

                other->erase();
                delete other;
                break;
            }
        }
    } else {
        at = mDefineStack.mRoot->mPrev;
        bucket = linker;
    }

    at->insertAfter(linker);

    if (mDeployInfoNum != 0) {
        linker->mIsEnabled = false;
        linker->mIsResolvable = false;

        for (u32 i = 0; i < mDeployInfoNum; i++) {
            if (linker->mName->compare(mDeployInfos[i].mName, mDeployInfos[i].mLength) == 0) {
                linker->mIsEnabled = true;
                linker->mIsResolvable = true;
                break;
            }
        }
    }

    return linker;
}

/**
 * Finds the macro whose name matches a clause.
 * @param pClause first clause of the name
 * @param hash hash of the name
 * @return the macro definition, or nullptr
 */
const DefineLinker* Resolver::compareMacro(const Clause* pClause, u32 hash) const {
    u32 index = hash & 0xff;
    DefineLinker* linker = mDefineStack.mTable[index];

    if (!linker) {
        return nullptr;
    }

    for (DefineLinker* root = mDefineStack.mRoot; root != linker; linker = linker->mNext) {
        if ((linker->mHash & 0xff) != index) {
            break;
        }

        if (linker->mHash == hash && *pClause->mBegin == *linker->mName->mBegin &&
            pClause->compare(*linker->mName, 0) == 0) {
            return linker;
        }
    }

    return nullptr;
}

/**
 * Counts the macros currently defined.
 * @return number of defined macros
 */
u32 Resolver::calcMacroNum() const {
    u32 num = 0;

    for (DefineLinker* linker = mDefineStack.mRoot->mNext; linker != mDefineStack.mRoot;
         linker = linker->mNext) {
        num++;
    }

    return num;
}

/**
 * Finds the macro whose name matches the words starting at a clause.
 * @param pClause first clause of the name
 * @return the macro definition, or nullptr
 */
const DefineLinker* Resolver::compareMacro(const Clause* pClause) const {
    u32 hash = 0;

    for (const Clause* c = pClause; c->isWord(); c = c->mNext) {
        hash = c->calcHash(hash);
    }

    return compareMacro(pClause, hash);
}

Clause* Resolver::replaceMacro(Clause* pClause, const DefineLinker* pLinker, bool isConcat) const {
    Clause* root = new (mUnitHeap, 4) Clause();
    Clause* prev = pClause->mPrev;

    Clause* end = pClause;

    while (end->isWord()) {
        end = end->mNext;
    }

    if (pLinker->mValueBegin) {
        Clause::constIterator valueBegin(pLinker->mValueBegin);
        Clause::constIterator valueEnd(pLinker->mValueEnd);
        root->insertListAfter(Clause::clone(mUnitHeap, valueBegin, valueEnd, false));
    }

    Clause* paren = pLinker->mName->mNext;

    if (paren->mType == Clause::cType_LParen) {
        Clause* c = paren;

        do {
            c = c->mNext;
        } while (c->isSpace());

        if (c->mType == Clause::cType_RParen) {
            do {
                end = end->mNext;
            } while (end->isSpace());
            end = end->mNext;
        } else {
            while (end->mType == Clause::cType_Space || end->mType == Clause::cType_LineFeed) {
                end = end->mNext;
            }

            do {
                end = end->mNext;
            } while (end->mType == Clause::cType_BackSlash || end->isSpace());

            MacroArgument args[64] = {};
            u32 argNum = 0;
            bool isContinue;

            do {
                s32 depth = end->mType == Clause::cType_LParen ? 1 : 0;
                Clause* arg = end;

                for (;;) {
                    arg = arg->mNext;
                    u32 type = arg->mType;

                    if (depth != 0) {
                        if (type == Clause::cType_LParen) {
                            depth++;
                        } else {
                            depth -= type == Clause::cType_RParen;
                        }

                        continue;
                    }

                    if (type == Clause::cType_LParen) {
                        depth = 1;
                        continue;
                    }

                    if (type == Clause::cType_Comma) {
                        isContinue = true;
                        break;
                    }

                    if (type == Clause::cType_RParen) {
                        isContinue = false;
                        break;
                    }
                }

                MacroArgument& info = args[argNum];
                info.mBegin = end;
                info.mEnd = arg->mPrev;
                end = skipSpace(end);
                info.mBegin = end;
                Clause* argEnd = info.mEnd;

                while (argEnd->isSpace()) {
                    argEnd = argEnd->mPrev;
                }

                info.mEnd = argEnd;
                end = arg->mNext;
                argNum++;
            } while (isContinue);

            u32 index = 0;

            for (Clause* c = paren; c != pLinker->mArgEnd;) {
                if (c->mType != Clause::cType_Word) {
                    c = c->mNext;
                    continue;
                }

                args[index].mName = c;
                u32 hash = 0;

                for (Clause* word = c; word->isWord(); word = word->mNext) {
                    hash = word->calcHash(hash);
                }

                args[index].mHash = hash;

                while (c->isWord()) {
                    c = c->mNext;
                }

                if (index == argNum - 1) {
                    break;
                }

                index++;
            }

            for (Clause* c = root->mNext; c != root;) {
                if (c->mType != Clause::cType_Word) {
                    c = c->mNext;
                    continue;
                }

                u32 hash = 0;

                for (Clause* word = c; word->isWord(); word = word->mNext) {
                    hash = word->calcHash(hash);
                }

                u32 i;

                for (i = 0; i < argNum; i++) {
                    if (hash == args[i].mHash && c->compare(*args[i].mName, 0) == 0) {
                        Clause* before = c->mPrev;
                        Clause* next = c->mNext;
                        Clause::constIterator argBegin(args[i].mBegin);
                        Clause::constIterator argEnd(args[i].mEnd);
                        before->insertListAfter(Clause::clone(mUnitHeap, argBegin, argEnd, true));
                        removeClause(c);
                        c = next;
                        break;
                    }
                }

                if (i == argNum) {
                    while (c->isWord()) {
                        c = c->mNext;
                    }
                }
            }
        }
    }

    if (isConcat) {
        for (Clause* c = root->mNext; c != root; c = c->mNext) {
            if (c->mType != Clause::cType_DoubleSharp || c->mPrev == root) {
                continue;
            }

            Clause* left = c;

            do {
                left = left->mPrev;
            } while (left->isSpace());
            Clause* right = c;

            do {
                right = right->mNext;
            } while (right->isSpace());
            removeClause(left->mNext, right, false);
            c = right->mPrev;
        }
    }

    Clause* list = root->isAlone() ? nullptr : root->mNext;
    removeClause(root);
    removeClause(pClause, end, false);

    Clause* result = prev->mNext;

    if (!list) {
        return result;
    }

    Clause* last = list->mPrev;
    result->insertListBefore(list);

    Clause* next = result;
    next = skipSpace(next);

    if (next->mType != Clause::cType_LineFeed && next->mType != Clause::cType_BlockCommentLF) {
        while (last->mType == Clause::cType_LineComment) {
            removeClause(last);
            last = result->mPrev;
        }
    }

    Clause* lineHead = prev;

    while (lineHead->mType != Clause::cType_LineFeed && lineHead->mType != Clause::cType_End) {
        lineHead = lineHead->mPrev;
    }

    Clause* directive = lineHead->mNext;

    if (directive->mType == Clause::cType_Sharp) {
        do {
            directive = directive->mNext;
        } while (directive->isSpace());

        if (directive->compare("if", 2) == 0 || directive->compare("elif", 4) == 0) {
            for (Clause* c = directive->mNext; c != result;) {
                Clause* cNext = c->mNext;

                if (c->mType == Clause::cType_BlockCommentLF || c->mType == Clause::cType_LineFeed) {
                    removeClause(c);
                }

                c = cNext;
            }
        }
    }

    return result;
}

Clause* Resolver::resolveMacro(Clause* pClause, bool isExpandDefined, bool isConcat) const {
    s32 macroLimit = -1;
    Clause* head = pClause->mPrev;
    Clause* end = pClause->mNext;
    u32 count = 0;
    u32 prevCount;

    do {
        prevCount = count;

        for (Clause* c = head->mNext; c != end; c = c->mNext) {
            if (c->mType != Clause::cType_Word) {
                continue;
            }

            if (c->compare("defined", 7) != 0) {
                const DefineLinker* linker = compareMacro(c);

                if (linker && linker->mIsEnabled) {
                    if (macroLimit < 0) {
                        u32 num = calcMacroNum();
                        macroLimit = num * num;
                    }

                    end = replaceMacro(c, linker, isConcat);
                    count++;
                    break;
                }

                continue;
            }

            Clause* close = c->mNext;
            bool isDefined = false;
            bool isResolvable = true;

            while (close->mType != Clause::cType_RParen) {
                if (isResolvable && !isDefined && close->mType == Clause::cType_Word) {
                    const DefineLinker* linker = compareMacro(close);

                    if (linker) {
                        isResolvable = linker->mIsResolvable;
                        isDefined = true;
                    } else if (mDeployInfoNum == 0) {
                        isResolvable = true;
                    } else {
                        isResolvable = false;

                        for (u32 i = 0; i < mDeployInfoNum; i++) {
                            if (close->compare(mDeployInfos[i].mName, mDeployInfos[i].mLength) ==
                                0) {
                                isResolvable = mDeployInfos[i].mIsActive;
                                break;
                            }
                        }
                    }
                }

                close = close->mNext;
            }

            for (Clause* it = c; it != close; it = it->mNext) {
                if (it == end) {
                    end = close;
                    break;
                }
            }

            if (end == close) {
                end = close->mNext;
            }

            if (isResolvable && isExpandDefined) {
                Clause* before = c->mPrev;
                Clause* value = new (mUnitHeap)
                    Clause(Clause::cType_Int, isDefined ? "1" : "0", (isDefined ? "1" : "0") + 1);
                value->mFlag |= 1;
                before->insertAfter(value);
                removeClause(c, close, true);
                c = before;
            }
        }
    } while (count != prevCount);
    return end;
}

SyntaxTree* Resolver::resolveExpression(Clause* pBegin, Clause* pEnd, bool isStatic) {
    Clause* head = pBegin->mPrev;
    Clause::constIterator begin(pBegin);
    Clause::constIterator end(pEnd);
    Clause* backup = Clause::clone(mUnitHeap, begin, end, false);

    if (!isStatic) {
        for (Clause* c = head->mNext; c != pEnd; c = c->mNext) {
            if (c->mType != Clause::cType_Word) {
                continue;
            }

            Clause* zero = new (mUnitHeap) Clause(Clause::cType_Int, "0", "0" + 1);
            zero->mFlag |= 1;
            c->mPrev->insertAfter(zero);

            while (c != pEnd && c->isWord()) {
                Clause* next = c->mNext;
                c->erase();
                c = next;
            }

            c = zero;
        }
    }

    SyntaxTree* tree = mEvaluator.resolve(pBegin, pEnd, !isStatic);

    if (tree) {
        removeClause(backup->mNext, backup, true);
    } else {
        head->insertListAfter(backup);
    }

    return tree;
}

Clause* Resolver::resolvePreprocessExpression(bool* pIsStatic, bool* pResult, Clause* pSharp,
                                              bool isStatic) {
    *pResult = false;
    *pIsStatic = false;
    if (pSharp->mType != Clause::cType_Sharp) {
        return nullptr;
    }

    Clause* directive = pSharp;

    do {
        directive = directive->mNext;
    } while (directive->isSpace());

    if (directive->mType != Clause::cType_Word) {
        return nullptr;
    }

    Clause* lineEnd = directive;

    do {
        lineEnd = lineEnd->mNext;

        if (lineEnd->mType == Clause::cType_End) {
            return nullptr;
        }
    } while (lineEnd->mType != Clause::cType_LineFeed);

    if (directive->compare("if", 2) == 0 || directive->compare("elif", 4) == 0) {
        Clause* exprBegin = directive->mNext;

        for (Clause* c = exprBegin; c->mType != Clause::cType_LineFeed;) {
            c = resolveMacro(c, mFlags.isOn(0x10), true);
        }

        Clause::constIterator begin(exprBegin);
        Clause::constIterator end(lineEnd);
        Clause* expr = Clause::clone(mUnitHeap, begin, end, true);
        Clause* exprLast = expr->mPrev;
        SyntaxTree* tree = resolveExpression(expr, exprLast, isStatic);
        removeClause(exprLast, exprLast->mPrev, true);

        if (tree) {
            *pResult = tree->evaluate() != 0.0;
            *pIsStatic = tree->mIsValid;
            tree->removeClauseRecursive(mRemoveDelegate);
            delete tree;
            return lineEnd;
        }

        Clause* retry = Clause::clone(mUnitHeap, begin, end, true);
        Clause* retryLast = retry->mPrev;

        if (retry != retryLast) {
            bool hasDefined = false;

            for (Clause* c = retry; c != retryLast; c = c->mNext) {
                if (c->mType == Clause::cType_Word && c->compare("defined", 7) == 0) {
                    c = c->mNext;
                    removeClause(c->mPrev, c, false);
                    hasDefined = true;
                }
            }

            if (hasDefined) {
                SyntaxTree* definedTree = resolveExpression(retry, retryLast, true);
                removeClause(retryLast, retryLast->mPrev, true);
                *pResult = definedTree->evaluate() != 0.0;
                *pIsStatic = definedTree->mIsValid;
                definedTree->removeClauseRecursive(mRemoveDelegate);
                delete definedTree;
                return lineEnd;
            }
        }

        dumpClauses(pSharp, lineEnd);
        *pResult = false;
        *pIsStatic = false;
        return lineEnd;
    }

    if (!mFlags.isOn(0x10)) {
        return nullptr;
    }

    if (directive->compare("ifdef", 5) != 0 && directive->compare("ifndef", 6) != 0) {
        return nullptr;
    }

    Clause* name = directive;

    do {
        name = name->mNext;

        if (name == lineEnd) {
            return lineEnd;
        }
    } while (name->mType != Clause::cType_Word);

    if (directive->compare("ifdef", 5) == 0) {
        const DefineLinker* linker = compareMacro(name);

        if (linker) {
            *pResult = true;
            *pIsStatic = linker->mIsResolvable;
            return lineEnd;
        }

        *pResult = false;
    } else if (directive->compare("ifndef", 6) == 0) {
        const DefineLinker* linker = compareMacro(name);

        if (linker) {
            *pResult = false;
            *pIsStatic = linker->mIsResolvable;
            return lineEnd;
        }

        *pResult = true;
    }

    *pIsStatic = mDeployInfoNum == 0;
    for (u32 i = 0; i < mDeployInfoNum; i++) {
        if (name->compare(mDeployInfos[i].mName, mDeployInfos[i].mLength) == 0) {
            *pIsStatic = mDeployInfos[i].mIsActive;
            break;
        }
    }

    return lineEnd;
}

bool Resolver::resolvePreprocessBlock(Clause* pSharp, bool isStatic) {
    bool result = false;
    bool isResolved = false;
    Clause* lineEnd = resolvePreprocessExpression(&isResolved, &result, pSharp, isStatic);

    if (!lineEnd) {
        return false;
    }

    if (isResolved) {
        Clause* next = findPreprocessEnd(lineEnd, mRoot, false);

        if (result) {
            Clause* endif = findPreprocessEnd(next, pSharp, true);

            do {
                endif = endif->mNext;
            } while (endif->isSpace());
            removeClause(next, endif, true);
        } else {
            removeClause(lineEnd->mNext, next, false);
        }

        removeClause(pSharp, lineEnd, true);
    }

    return isResolved;
}

/**
 * Evaluates a conditional directive and reports the block that is kept.
 * @param ppBlockBegin receives the start of the kept block, or nullptr
 * @param ppBlockEnd receives the end of the kept block, or nullptr
 * @param ppPartEnd receives the end of the whole conditional part
 * @param pSharp '#' clause of the directive
 * @param isStatic whether the expression is resolved statically
 * @return whether the condition could be resolved
 */
bool Resolver::markPreprocessBlock(Clause** ppBlockBegin, Clause** ppBlockEnd, Clause** ppPartEnd,
                                   Clause* pSharp, bool isStatic) {
    bool result = false;
    bool isResolved = false;
    Clause* lineEnd = resolvePreprocessExpression(&isResolved, &result, pSharp, isStatic);

    if (!lineEnd) {
        return false;
    }

    Clause* next = findPreprocessEnd(lineEnd, pSharp, false);
    Clause* blockBegin = nullptr;
    Clause* blockEnd = nullptr;
    Clause* partEnd = next;

    if (isResolved && result) {
        Clause* endif = findPreprocessEnd(next, pSharp, true);

        do {
            endif = endif->mNext;
        } while (endif->isSpace());
        blockBegin = lineEnd;
        blockEnd = next;
        partEnd = endif;
    }

    *ppBlockBegin = blockBegin;
    *ppBlockEnd = blockEnd;
    *ppPartEnd = partEnd;
    return isResolved;
}

bool Resolver::resolveProgramBlock(Clause* pIf, bool* pResult) {
    if (pIf->mType != Clause::cType_Word || pIf->compare("if", 2) != 0) {
        return false;
    }

    Clause* open = pIf->mNext;
    Clause* close = open;
    close = skipSpace(close);

    if (close->isOpenBracket()) {
        s32 depth = 0;

        while (depth != 1 || !close->isCloseBracket()) {
            depth += close->isOpenBracket() ? 1 : -close->isCloseBracket();

            do {
                close = close->mNext;
            } while (close->isSpace());

            if (close == open->mPrev || close->mType == Clause::cType_End) {
                return false;
            }
        }
    }

    if (!close) {
        return false;
    }

    Clause* exprEnd = close->mNext;

    for (Clause* c = open; c != exprEnd;) {
        c = resolveMacro(c, mFlags.isOn(0x10), true);
    }

    SyntaxTree* tree;
    bool hasUnresolved = false;

    for (Clause* c = open; c != exprEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Question) {
            Clause* begin = findTernaryBegin(c);
            Clause* start = begin;

            do {
                start = start->mNext;
            } while (start->isSpace());

            if (resolveTernaryBlock(start)) {
                c = begin;
            }
        }

        if (c->mType != Clause::cType_Word) {
            continue;
        }

        if (c->compare("false", 5) == 0) {
            c->set(Clause::cType_Int, "0", "0" + 1);
            c->mFlag |= 1;
        } else if (c->compare("true", 4) == 0) {
            c->set(Clause::cType_Int, "1", "1" + 1);
            c->mFlag |= 1;
        } else {
            if (!mFlags.isOn(8)) {
                return false;
            }

            hasUnresolved = true;
        }
    }

    if (hasUnresolved) {
        Clause::constIterator begin(open);
        Clause::constIterator end(exprEnd);
        Clause* expr = Clause::clone(mUnitHeap, begin, end, true);
        Clause* exprLast = expr->mPrev;
        tree = resolveExpression(expr, exprLast, true);
        removeClause(exprLast, exprLast->mPrev, true);

        if (!tree) {
            return false;
        }

        removeClause(open, exprEnd, false);
    } else {
        tree = resolveExpression(open, exprEnd, false);

        if (!tree) {
            return false;
        }
    }

    f64 value = tree->evaluate();
    bool isStatic = tree->mIsValid;
    bool result = value != 0.0;

    if (!isStatic) {
        pIf->insertListAfter(tree->construct(mUnitHeap, mFrameHeap));
    }

    tree->removeClauseRecursive(mRemoveDelegate);
    delete tree;

    if (isStatic) {
        Clause* statementEnd = exprEnd;
        statementEnd = skipSpace(statementEnd);

        if (statementEnd->isOpenBracket()) {
            s32 depth = 0;

            while (depth != 1 || !statementEnd->isCloseBracket()) {
                depth += statementEnd->isOpenBracket() ? 1 : -statementEnd->isCloseBracket();

                do {
                    statementEnd = statementEnd->mNext;
                } while (statementEnd->isSpace());
            }
        }

        if (statementEnd->mType != Clause::cType_Semicolon && !statementEnd->isCloseBracket()) {
            do {
                statementEnd = statementEnd->mNext;
            } while (statementEnd->mType != Clause::cType_Semicolon);
        }

        Clause* bodyBegin = exprEnd->mNext;
        Clause* after = statementEnd->mNext;
        removeClause(pIf, bodyBegin, false);

        Clause* next = after;
        next = skipSpace(next);

        if (value != 0.0) {
            if (next->mType == Clause::cType_Word && next->compare("else", 4) == 0) {
                Clause* elseEnd = skipElseBlock(after, pIf->mPrev);
                removeClause(after, elseEnd, false);
            }

            result = true;
        } else {
            if (next->mType == Clause::cType_Word && next->compare("else", 4) == 0) {
                after = next->mNext;
            }

            removeClause(bodyBegin, after, false);
            result = false;
        }
    }

    if (pResult) {
        *pResult = result;
    }

    return isStatic;
}

bool Resolver::resolveTernaryBlock(Clause* pClause) {
    Clause* prev = pClause->mPrev;

    Clause* question = pClause;

    for (;;) {
        if (question->isSpace()) {
            question = question->mNext;
            continue;
        }

        if (question->isOpenBracket()) {
            s32 depth = 0;

            while (depth != 1 || !question->isCloseBracket()) {
                depth += question->isOpenBracket() ? 1 : -question->isCloseBracket();

                do {
                    question = question->mNext;
                } while (question->isSpace());
            }

            question = question->mNext;
            continue;
        }

        u32 type = question->mType;

        if (type == Clause::cType_Dot) {
            question = question->mNext;
            continue;
        }

        if (type == Clause::cType_Colon || type == Clause::cType_Question) {
            break;
        }

        if (question->isOperator() || question->isWord()) {
            question = question->mNext;
            continue;
        }

        break;
    }

    if (!question) {
        return false;
    }

    for (Clause* c = pClause; c != question;) {
        c = resolveMacro(c, mFlags.isOn(0x10), true);
    }

    SyntaxTree* tree;
    bool hasUnresolved = false;
    s32 nest = 1;

    for (Clause* c = pClause; c != question; c = c->mNext) {
        if (c->mType == Clause::cType_Question) {
            if (nest != 0) {
                nest--;
                continue;
            }

            Clause* begin = findTernaryBegin(c);
            Clause* start = begin;

            do {
                start = start->mNext;
            } while (start->isSpace());

            if (resolveTernaryBlock(start)) {
                c = begin;
            }
        }

        if (c->mType != Clause::cType_Word) {
            continue;
        }

        if (c->compare("false", 5) == 0) {
            c->set(Clause::cType_Int, "0", "0" + 1);
            c->mFlag |= 1;
        } else if (c->compare("true", 4) == 0) {
            c->set(Clause::cType_Int, "1", "1" + 1);
            c->mFlag |= 1;
        } else {
            if (!mFlags.isOn(8)) {
                return false;
            }

            hasUnresolved = true;
        }
    }

    if (hasUnresolved) {
        Clause::constIterator begin(pClause);
        Clause::constIterator end(question);
        Clause* expr = Clause::clone(mUnitHeap, begin, end, true);
        Clause* exprLast = expr->mPrev;
        tree = resolveExpression(expr, exprLast, true);
        removeClause(exprLast, exprLast->mPrev, true);

        if (!tree) {
            return false;
        }

        removeClause(pClause, question, false);
    } else {
        tree = resolveExpression(pClause, question, false);

        if (!tree) {
            return false;
        }
    }

    f64 value = tree->evaluate();
    bool isStatic = tree->mIsValid;

    if (!isStatic) {
        prev->insertListAfter(tree->construct(mUnitHeap, mFrameHeap));
    }

    tree->removeClauseRecursive(mRemoveDelegate);
    delete tree;

    if (!isStatic) {
        return false;
    }

    Clause* trueBegin = question->mNext;
    Clause* colon = trueBegin;
    s32 depth = 0;

    for (;; colon = colon->mNext) {
        colon = skipSpace(colon);

        if (colon->isOpenBracket()) {
            s32 bracketDepth = 0;

            while (bracketDepth != 1 || !colon->isCloseBracket()) {
                bracketDepth += colon->isOpenBracket() ? 1 : -colon->isCloseBracket();

                do {
                    colon = colon->mNext;
                } while (colon->isSpace());
            }

            continue;
        }

        u32 type = colon->mType;

        if (type == Clause::cType_Colon) {
            if (depth == 0) {
                break;
            }

            depth--;
        } else if (type == Clause::cType_Question) {
            depth++;
        } else if (colon->isCloseBracket() || type == Clause::cType_Comma ||
                   type == Clause::cType_Semicolon) {
            break;
        }
    }

    Clause* falseBegin = colon->mNext;

    if (value != 0.0) {
        Clause* falseEnd = falseBegin;
        s32 falseDepth = 0;

        for (;; falseEnd = falseEnd->mNext) {
            falseEnd = skipSpace(falseEnd);

            if (falseEnd->isOpenBracket()) {
                s32 bracketDepth = 0;

                while (bracketDepth != 1 || !falseEnd->isCloseBracket()) {
                    bracketDepth += falseEnd->isOpenBracket() ? 1 : -falseEnd->isCloseBracket();

                    do {
                        falseEnd = falseEnd->mNext;
                    } while (falseEnd->isSpace());
                }

                continue;
            }

            u32 type = falseEnd->mType;

            if (type == Clause::cType_Colon) {
                if (falseDepth == 0) {
                    break;
                }

                falseDepth--;
            } else if (type == Clause::cType_Question) {
                falseDepth++;
            } else if (falseEnd->isCloseBracket() || type == Clause::cType_Comma ||
                       type == Clause::cType_Semicolon) {
                break;
            }
        }

        removeClause(colon, falseEnd, false);
    } else {
        removeClause(trueBegin, falseBegin, false);
    }

    removeClause(prev->mNext, question, true);
    return true;
}

/**
 * Sets up heaps, text and the define stack.
 * @param pHeap heap for definitions
 * @param pUnitHeap heap for clauses
 * @param pFrameHeap heap for temporary clauses
 * @param pText source text
 * @param pRoot root of the clause list
 * @param pDefineRoot root of the definition list
 * @param pRemoveDelegate delegate that removes a clause
 */
void Resolver::initialize(sead::Heap* pHeap, sead::Heap* pUnitHeap, sead::Heap* pFrameHeap,
                          const char* pText, Clause* pRoot, DefineLinker* pDefineRoot,
                          sead::AnyDelegate1Const<Clause*>* pRemoveDelegate) {
    mHeap = pHeap;
    mUnitHeap = pUnitHeap;
    mFrameHeap = pFrameHeap;
    mText = pText;
    mRoot = pRoot;
    mRemoveDelegate = pRemoveDelegate;
    mEvaluator.initialize(pHeap, pUnitHeap, pRemoveDelegate);
    mDefineStack.mRoot = pDefineRoot;
    restructDefineStack(&mDefineStack);
}

/**
 * Sets the macros whose values replace the values given in the source.
 * @param pInfos replacement macros
 * @param num number of replacement macros
 */
void Resolver::setMacroReplaceInfo(const MacroReplaceInfo* pInfos, u32 num) {
    mReplaceInfos = pInfos;
    mReplaceInfoNum = num;
}

/**
 * Sets the macros that are deployed and marks all of them active.
 * @param pInfos deployed macros
 * @param num number of deployed macros
 */
void Resolver::setMacroDeployInfo(const MacroDeployInfo* pInfos, u32 num) {
    mDeployInfos = const_cast<MacroDeployInfo*>(pInfos);
    mDeployInfoNum = num;

    for (u32 i = 0; i < mDeployInfoNum; i++) {
        mDeployInfos[i].mIsActive = true;
    }
}

void Resolver::resolveStaticBranch(Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin; c != pEnd;) {
        u32 type = c->mType;

        if (type == Clause::cType_Word) {
            if (c->compare("if", 2) == 0) {
                Clause* prev = c->mPrev;

                if (resolveProgramBlock(c, nullptr)) {
                    c = prev;
                } else {
                    Clause* body = c->mNext;
                    body = skipSpace(body);

                    if (body->isOpenBracket()) {
                        s32 depth = 0;

                        while (depth != 1 || !body->isCloseBracket()) {
                            depth += body->isOpenBracket() ? 1 : -body->isCloseBracket();

                            do {
                                body = body->mNext;
                            } while (body->isSpace());
                        }
                    }

                    do {
                        body = body->mNext;
                    } while (body->isSpace());

                    Clause* bodyEnd = body;
                    bool isBlock = body->isOpenBracket();

                    if (isBlock) {
                        s32 depth = 0;

                        for (Clause* it = body;;) {
                            if (depth == 1 && it->isCloseBracket()) {
                                bodyEnd = it;
                                break;
                            }

                            depth += it->isOpenBracket() ? 1 : -it->isCloseBracket();

                            do {
                                it = it->mNext;
                            } while (it->isSpace());
                            bodyEnd = nullptr;

                            if (it == pEnd || it->mType == Clause::cType_End) {
                                break;
                            }
                        }

                        if (isBlock) {
                            body = body->mNext;
                        }
                    }

                    if (bodyEnd->mType != Clause::cType_Semicolon && !bodyEnd->isCloseBracket()) {
                        do {
                            bodyEnd = bodyEnd->mNext;
                        } while (bodyEnd->mType != Clause::cType_Semicolon);
                    }

                    bodyEnd = skipSpace(bodyEnd);
                    resolveStaticBranch(body, bodyEnd);

                    Clause* next = bodyEnd;

                    do {
                        next = next->mNext;
                    } while (next->isSpace());

                    Clause* last = next;

                    while (next->mType == Clause::cType_Word && next->compare("else", 4) == 0) {
                        Clause* elseClause = next;
                        Clause* elseBody = elseClause;

                        do {
                            elseBody = elseBody->mNext;
                        } while (elseBody->isSpace());

                        if (elseBody->compare("if", 2) == 0) {
                            Clause* elseEnd = skipElseBlock(elseClause, pEnd);
                            Clause* before = elseBody->mPrev;
                            elseEnd = skipSpace(elseEnd);
                            resolveStaticBranch(elseBody, elseEnd);
                            before = skipSpace(before);
                            last = elseEnd;

                            if (before == elseEnd) {
                                Clause* elsePrev = elseClause->mPrev;
                                removeClause(elseClause);
                                last = elsePrev->mNext;
                            }

                            break;
                        }

                        Clause* elseBodyEnd = elseBody;
                        elseBodyEnd = skipSpace(elseBodyEnd);

                        if (elseBodyEnd->isOpenBracket()) {
                            s32 depth = 0;

                            while (depth != 1 || !elseBodyEnd->isCloseBracket()) {
                                depth += elseBodyEnd->isOpenBracket() ? 1
                                                                      : -elseBodyEnd->isCloseBracket();
                                do {
                                    elseBodyEnd = elseBodyEnd->mNext;
                                } while (elseBodyEnd->isSpace());
                            }
                        } else if (!elseBodyEnd->isCloseBracket()) {
                            for (; elseBodyEnd; elseBodyEnd = elseBodyEnd->mNext) {
                                if (elseBodyEnd->mType == Clause::cType_Semicolon) {
                                    break;
                                }

                                if (elseBodyEnd->mType == Clause::cType_LBrace) {
                                    break;
                                }
                            }
                        }
                        elseBodyEnd = skipSpace(elseBodyEnd);
                        resolveStaticBranch(elseBody->mNext, elseBodyEnd);

                        do {
                            elseBodyEnd = elseBodyEnd->mNext;
                        } while (elseBodyEnd->isSpace());
                        last = elseBodyEnd;
                        break;
                    }

                    c = last->mPrev;
                }
            } else {
                c->compare("else", 4);
            }
        } else if (type == Clause::cType_Sharp) {
            Clause* directive = c->mNext;
            directive = skipSpace(directive);

            if (directive->compare("define", 6) == 0) {
                c = findDefineEnd(directive, pEnd);
            } else {
                c = directive;

                while (c->mType != Clause::cType_LineFeed && c->mType != Clause::cType_End) {
                    c = c->mNext;
                }
            }
        } else if (type == Clause::cType_Question) {
            Clause* begin = findTernaryBegin(c);
            Clause* start = begin;

            do {
                start = start->mNext;
            } while (start->isSpace());

            if (resolveTernaryBlock(start)) {
                c = begin;
            }
        }

        do {
            c = c->mNext;
        } while (c->isSpace());
    }
}

void Resolver::executePart(Clause* pSharp, Clause* pEnd, Clause* pSkip) {
    if (pSharp == pEnd) {
        return;
    }

    for (Clause* c = pSharp;;) {
        Clause* directive = skipSpace(c->mNext);

        if (directive->compare("if", 2) == 0 || directive->compare("elif", 4) == 0 ||
            directive->compare("ifdef", 5) == 0 || directive->compare("ifndef", 6) == 0 ||
            directive->compare("else", 4) == 0) {
            Clause* next = findPreprocessEnd(directive, pEnd, false);

            if (!next) {
                next = pEnd;
            }

            if (c != pSkip) {
                pushDefineStack();
                executeImpl(directive, next, false);
                popDefineStack();
            }

            c = next->mPrev;
        } else if (directive->compare("endif", 5) == 0) {
            return;
        }

        do {
            c = c->mNext;
        } while (c->isSpace());

        if (c == pEnd) {
            return;
        }
    }
}

void Resolver::executeImpl(Clause* pBegin, Clause* pEnd, bool isInner) {
    bool isOuter = !isInner;

    for (Clause* c = pBegin; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Word) {
            if (mFlags.isOn(2)) {
                c = resolveMacro(c, mFlags.isOn(0x10), true)->mPrev;
            }

            continue;
        }

        if (c->mType != Clause::cType_Sharp) {
            continue;
        }

        Clause* directive = c->mNext;
        directive = skipSpace(directive);

        if (directive->mType != Clause::cType_Word) {
            continue;
        }

        if (directive->compare("define", 6) == 0) {
            Clause* end = findDefineEnd(directive, pEnd);
            bool isReplaced = false;
            DefineLinker* linker = createDefineLinker(c, end, &isReplaced);

            if (linker->mIsEnabled || isReplaced) {
                if (!mFlags.isOn(2) || isOuter) {
                    removeClause(c, end, false);

                    if (end == pEnd) {
                        return;
                    }

                    c = end;
                    continue;
                }

                if (isReplaced) {
                    removeClause(c, end, false);
                    Clause* root = linker->mRoot;
                    Clause::constIterator begin(root->mNext);
                    Clause::constIterator rootEnd(root);
                    Clause* before = end->mPrev;
                    before->insertListAfter(Clause::clone(mUnitHeap, begin, rootEnd, false));

                    if (end == pEnd) {
                        return;
                    }

                    c = end;
                    continue;
                }
            }

            if (mFlags.isOn(2)) {
                Clause* body = directive;

                do {
                    body = body->mNext;
                } while (body->isSpace());

                while (body->mType == Clause::cType_Word) {
                    body = body->mNext;
                }

                if (body->mType == Clause::cType_LParen) {
                    do {
                        body = body->mNext;
                    } while (body->mType != Clause::cType_RParen);
                    body = body->mNext;
                }

                if (body != end) {
                    for (Clause* it = body->mNext; it != end;) {
                        if (it->mType == Clause::cType_Word && it->compare(*linker->mName, 0) == 0) {
                            dumpClauses(it, end);

                            while (it->mType == Clause::cType_Word) {
                                it = it->mNext;
                            }
                        } else {
                            it = resolveMacro(it, mFlags.isOn(0x10), false);
                        }
                    }

                    for (Clause* it = body->mNext; it != end; it = it->mNext) {
                        if (it->mType != Clause::cType_LineFeed) {
                            continue;
                        }

                        Clause* prev = it->mPrev;

                        if (prev->mType == Clause::cType_BackSlash) {
                            continue;
                        }

                        Clause* backSlash =
                            new (mUnitHeap) Clause(Clause::cType_BackSlash, "\\", "\\" + 1);
                        backSlash->mFlag |= 1;
                        prev->insertListAfter(backSlash);
                    }
                }
            }

            if (end == pEnd) {
                return;
            }

            c = end;
            continue;
        }

        if (!mFlags.isOn(1)) {
            if (directive->compare("if", 2) == 0 || directive->compare("elif", 4) == 0 ||
                directive->compare("ifdef", 5) == 0 || directive->compare("ifndef", 6) == 0) {
                Clause* blockBegin = nullptr;
                Clause* blockEnd = nullptr;
                Clause* partEnd = nullptr;
                markPreprocessBlock(&blockBegin, &blockEnd, &partEnd, c, mFlags.isOn(0x20));
                Clause* skip = nullptr;

                if (blockBegin) {
                    skip = blockBegin;

                    while (skip->mType != Clause::cType_Sharp) {
                        skip = skip->mPrev;
                    }
                }

                executePart(c, partEnd, skip);

                if (blockBegin && blockEnd) {
                    executeImpl(blockBegin, blockEnd, false);
                }

                c = partEnd->mPrev;
            } else if (directive->compare("else", 4) == 0 ||
                       directive->compare("endif", 5) == 0 ||
                       directive->compare("undef", 5) != 0) {
                c = directive;
            } else {
                Clause* p = directive;

                while (p->isSpace()) {
                    p = p->mPrev;
                }

                Clause* name = directive;

                do {
                    name = name->mNext;
                } while (name->isSpace());
                const_cast<DefineLinker*>(compareMacro(name))->mIsEnabled = false;
                c = name->mPrev;
            }

            continue;
        }

        if (directive->compare("if", 2) == 0 || directive->compare("elif", 4) == 0 ||
            directive->compare("ifdef", 5) == 0 || directive->compare("ifndef", 6) == 0) {
            Clause* prev = c->mPrev;

            if (resolvePreprocessBlock(c, mFlags.isOn(0x20))) {
                c = prev;
                continue;
            }

            if (directive->compare("elif", 4) == 0) {
                directive->set(Clause::cType_Word, directive->mBegin + 2, directive->mEnd);
            }

            Clause* partEnd = findPreprocessEnd(directive, pEnd, true);

            do {
                partEnd = partEnd->mNext;
            } while (partEnd->isSpace());

            do {
                partEnd = partEnd->mNext;
            } while (partEnd->isSpace());
            executePart(c, partEnd, nullptr);
            c = partEnd->mPrev;
        } else if (directive->compare("else", 4) == 0) {
            Clause* endif = findPreprocessEnd(directive, pEnd, true);
            Clause* endifWord = endif;

            do {
                endifWord = endifWord->mNext;
            } while (endifWord->isSpace());
            removeClause(endif, endifWord, true);
            Clause* prev = c->mPrev;
            removeClause(c, directive, true);
            c = prev;
        } else if (directive->compare("endif", 5) == 0) {
            Clause* prev = c->mPrev;
            removeClause(c, directive, true);
            c = prev;
        } else if (directive->compare("undef", 5) != 0) {
            c = directive;
        } else {
            Clause* p = directive;

            while (p->isSpace()) {
                p = p->mPrev;
            }

            Clause* first = p->mPrev;
            Clause* name = directive;

            do {
                name = name->mNext;
            } while (name->isSpace());
            const_cast<DefineLinker*>(compareMacro(name))->mIsEnabled = false;
            Clause* prev = first->mPrev;
            removeClause(first, name, true);
            c = prev;
        }
    }
}

/**
 * Resolves preprocessor directives and macros of the whole text.
 * @param isResolvePreprocess whether static preprocessor branches are resolved
 * @param isResolveMacro whether macros are expanded
 * @param isInner whether the text is part of another text
 * @param isStaticMacro whether macro existence checks are resolved statically
 * @param isKeepUnresolved whether unresolved words are kept in program branches
 * @param isResolveStaticBranch whether static program branches are resolved
 * @param isStaticExpression whether preprocessor expressions are resolved statically
 * @return always true
 */
bool Resolver::execute(bool isResolvePreprocess, bool isResolveMacro, bool isInner,
                       bool isStaticMacro, bool isKeepUnresolved, bool isResolveStaticBranch,
                       bool isStaticExpression) {
    mFlags.change(1, isResolvePreprocess);
    mFlags.change(2, isResolveMacro);
    mFlags.change(8, isKeepUnresolved);
    mFlags.change(0x10, isStaticMacro);
    mFlags.change(0x20, isStaticExpression);
    executeImpl(mRoot->mNext, mRoot, isInner);

    if (mFlags.isOn(2) && isResolveStaticBranch) {
        resolveStaticBranch(mRoot->mNext, mRoot);
    }

    return true;
}

}  // namespace agl::shtxt
