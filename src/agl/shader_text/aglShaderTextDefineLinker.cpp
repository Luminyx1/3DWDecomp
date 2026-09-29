#include "shader_text/aglShaderTextDefineLinker.h"

#include <basis/seadNew.h>

namespace agl::shtxt {

/**
 * Constructs an empty define linker.
 */
DefineLinker::DefineLinker() = default;

/**
 * Destroys the define linker and its clauses.
 */
DefineLinker::~DefineLinker() {
    clear();
}

/**
 * Deletes the clauses of the definition.
 */
void DefineLinker::clear() {
    if (!mRoot) {
        return;
    }
    Clause* root = mRoot;
    Clause* clause = root->mNext;
    if (clause != root) {
        Clause* next = clause->mNext;
        while (true) {
            clause->erase();
            delete clause;
            if (next == root) {
                break;
            }
            clause = next;
            next = clause->mNext;
        }
    }
    delete mRoot;
    mRoot = nullptr;
    mName = nullptr;
    mArgEnd = nullptr;
    mValueBegin = nullptr;
    mValueEnd = nullptr;
}

/**
 * Sets the definition from a range of clauses.
 * @param pHeap heap for the copied clauses
 * @param pBegin first clause of the definition
 * @param pEnd end of the definition
 * @return whether the definition is valid
 */
bool DefineLinker::set(sead::Heap* pHeap, const Clause* pBegin, const Clause* pEnd) {
    return setImpl(pHeap, pBegin, pEnd, true);
}

/**
 * Sets the definition from a copy of a range of clauses.
 * @param pHeap heap for the copied clauses
 * @param pBegin first clause of the definition
 * @param pEnd end of the definition
 * @param calcHash whether the name hash is calculated
 * @return whether the definition is valid
 */
bool DefineLinker::setImpl(sead::Heap* pHeap, const Clause* pBegin, const Clause* pEnd,
                           bool calcHash) {
    Clause* root = new (pHeap) Clause();
    root->insertListAfter(Clause::clone(pHeap, Clause::constIterator(pBegin),
                                        Clause::constIterator(pEnd), false));
    for (Clause* clause = root->mNext; clause != root;) {
        Clause* next = clause->mNext;
        if (clause->mType == Clause::cType_BackSlash) {
            clause->erase();
            delete clause;
            next = next->mNext;
        }
        clause = next;
    }
    return setDirect(root, 2, calcHash);
}

/**
 * Sets the definition from a list of clauses that the linker takes ownership of.
 * @param pRoot list of clauses
 * @param index index of the word holding the macro name
 * @param calcHash whether the name hash is calculated
 * @return whether the name was found
 */
bool DefineLinker::setDirect(Clause* pRoot, u32 index, bool calcHash) {
    mRoot = pRoot;
    mName = nullptr;
    mArgEnd = nullptr;
    mValueBegin = nullptr;
    mValueEnd = nullptr;

    Clause* clause = pRoot->mNext;
    for (; clause != pRoot; clause = clause->mNext) {
        if (clause->mType == Clause::cType_Word && --index == 0) {
            break;
        }
    }
    if (clause == pRoot) {
        return false;
    }
    mName = clause;

    s32 depth = 0;
    bool hasArgs = false;
    for (clause = clause->mNext; clause != pRoot; clause = clause->mNext) {
        if (clause->mType == Clause::cType_RParen) {
            depth--;
        } else if (clause->mType == Clause::cType_LParen) {
            if (depth == 0) {
                hasArgs = !hasArgs;
                if (!hasArgs) {
                    break;
                }
            }
            depth++;
            hasArgs = true;
        } else if (depth == 0) {
            break;
        }
    }

    mArgEnd = clause;
    if (clause != pRoot) {
        while (clause->mType == Clause::cType_Space || clause->mType == Clause::cType_LineFeed) {
            clause = clause->mNext;
        }
        mValueBegin = clause;
        if (clause->mType != Clause::cType_None) {
            Clause* last = pRoot->mPrev;
            mValueEnd = last;
            while (last->mType == Clause::cType_Space) {
                last = last->mPrev;
            }
            mValueEnd = last;
            mValueEnd = last->mNext;
            if (mValueEnd) {
                goto end;
            }
        }
    }
    mValueBegin = nullptr;
    mValueEnd = nullptr;

end:
    if (calcHash) {
        mHash = mName->calcHash(0);
    }
    return true;
}

/**
 * Recalculates the hash of the macro name.
 */
void DefineLinker::updateHash() {
    mHash = mName->calcHash(0);
}

/**
 * Replaces the value of the definition with a copy of a range of clauses.
 * @param pHeap heap for the copied clauses
 * @param pBegin first clause of the new value
 * @param pEnd end of the new value
 * @param includeEnd whether pEnd itself is copied too
 */
void DefineLinker::replace(sead::Heap* pHeap, const Clause* pBegin, const Clause* pEnd,
                           bool includeEnd) {
    Clause* anchor;
    if (mValueBegin) {
        Clause* end = mValueEnd;
        anchor = mValueBegin->mPrev;
        for (Clause* clause = mValueBegin; clause != end;) {
            Clause* next = clause->mNext;
            clause->erase();
            delete clause;
            clause = next;
        }
        if (mValueEnd != mRoot) {
            mValueEnd->erase();
            delete mValueEnd;
        }
    } else {
        static const char cSpace[] = " ";
        anchor = new (pHeap) Clause(Clause::cType_Space, cSpace, cSpace + 1);
        mRoot->mPrev->insertAfter(anchor);
    }

    mValueBegin = Clause::clone(pHeap, Clause::constIterator(pBegin), Clause::constIterator(pEnd),
                                includeEnd);
    Clause* next = anchor->mNext;
    anchor->insertListAfter(mValueBegin);
    mValueEnd = next;
}

/**
 * Creates a copy of the definition.
 * @param pHeap heap for the new linker
 * @param pClauseHeap heap for the copied clauses
 * @return the copy
 */
DefineLinker* DefineLinker::clone(sead::Heap* pHeap, sead::Heap* pClauseHeap) const {
    DefineLinker* linker = new (pHeap) DefineLinker();
    linker->setImpl(pClauseHeap, mRoot->mNext, mRoot, false);
    linker->_38 = _38;
    linker->mHash = mHash;
    return linker;
}

/**
 * Creates a copy of every definition in the list that follows this one.
 * @param pHeap heap for the new linkers
 * @param pClauseHeap heap for the copied clauses
 * @return first copy, or nullptr if the list is empty
 */
DefineLinker* DefineLinker::cloneAll(sead::Heap* pHeap, sead::Heap* pClauseHeap) const {
    DefineLinker* head = nullptr;
    DefineLinker* last = nullptr;
    for (const DefineLinker* linker = mNext; linker != this; linker = linker->mNext) {
        DefineLinker* copy = linker->clone(pHeap, pClauseHeap);
        if (last) {
            last->insertAfter(copy);
        } else {
            head = copy;
        }
        last = copy;
    }
    return head;
}

}  // namespace agl::shtxt
