#include "shader_text/aglShaderTextExpressionEvaluator.h"

#include <basis/seadNew.h>

#include "shader_text/aglShaderTextClause.h"
#include "shader_text/aglShaderTextSyntaxTree.h"

namespace agl::shtxt {

/**
 * Constructs an evaluator without heaps.
 */
ExpressionEvaluator::ExpressionEvaluator() = default;

/**
 * Constructs an evaluator with its heaps and remove delegate.
 * @param pHeap heap for syntax trees
 * @param pClauseHeap heap for result clauses
 * @param pRemoveDelegate delegate that removes a clause
 */
ExpressionEvaluator::ExpressionEvaluator(sead::Heap* pHeap, sead::Heap* pClauseHeap,
                                         const sead::AnyDelegate1Const<Clause*>* pRemoveDelegate)
    : mHeap(pHeap), mClauseHeap(pClauseHeap), mRemoveDelegate(pRemoveDelegate) {}

/**
 * Sets the heaps and remove delegate.
 * @param pHeap heap for syntax trees
 * @param pClauseHeap heap for result clauses
 * @param pRemoveDelegate delegate that removes a clause
 */
void ExpressionEvaluator::initialize(sead::Heap* pHeap, sead::Heap* pClauseHeap,
                                     const sead::AnyDelegate1Const<Clause*>* pRemoveDelegate) {
    mHeap = pHeap;
    mClauseHeap = pClauseHeap;
    mRemoveDelegate = pRemoveDelegate;
}

/**
 * Finds the syntax tree whose value is a clause.
 * @param pLeashes syntax trees to search
 * @param pClause value clause to find
 * @return the matching leash, or nullptr
 */
SyntaxLeash* ExpressionEvaluator::findSyntaxLeash(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                  const Clause* pClause) const {
    for (auto& leash : *pLeashes) {
        if (leash.mClause == pClause) {
            return &leash;
        }
    }

    return nullptr;
}

Clause* ExpressionEvaluator::createBinaryOperatorSyntaxTree(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                            Clause* pClause) {
    Clause* value = new (mClauseHeap) Clause(Clause::cType_Int, "0", "0" + 1);
    SyntaxTree* tree = new (mHeap) SyntaxTree(value);
    SyntaxLeash* leash = pLeashes->emplaceBack();

    SyntaxLeash* left = findSyntaxLeash(pLeashes, pClause->mPrev);
    SyntaxLeash* right = findSyntaxLeash(pLeashes, pClause->mNext);

    SyntaxTree* leftTree = left->mTree;
    SyntaxTree* rightTree = right->mTree;
    tree->mRight = rightTree;
    tree->mOperator = pClause;
    tree->mLeft = leftTree;
    tree->mCenter = nullptr;
    leash->mTree = tree;
    leash->mClause = value;
    left->mTree = nullptr;
    left->mClause = nullptr;
    right->mTree = nullptr;
    right->mClause = nullptr;

    pLeashes->erase(pLeashes->indexOf(left));
    pLeashes->erase(pLeashes->indexOf(right));

    pClause->mPrev->mPrev->insertAfter(value);
    pClause->mPrev->erase();
    pClause->mNext->erase();
    pClause->erase();
    return value;
}

Clause* ExpressionEvaluator::createTernaryOperatorSyntaxTree(
    sead::ObjArray<SyntaxLeash>* pLeashes, Clause* pClause) {
    Clause* value = new (mClauseHeap) Clause(Clause::cType_Int, "0", "0" + 1);
    SyntaxTree* tree = new (mHeap) SyntaxTree(value);
    SyntaxLeash* leash = pLeashes->emplaceBack();

    Clause* condition = pClause->mPrev;
    Clause* trueValue = pClause->mNext;
    Clause* colon = trueValue->mNext;
    Clause* falseValue = colon->mNext;

    SyntaxLeash* left = findSyntaxLeash(pLeashes, condition);
    SyntaxLeash* center = findSyntaxLeash(pLeashes, trueValue);
    SyntaxLeash* right = findSyntaxLeash(pLeashes, falseValue);

    SyntaxTree* leftTree = left->mTree;
    SyntaxTree* centerTree = center->mTree;
    SyntaxTree* rightTree = right->mTree;
    tree->mRight = rightTree;
    tree->mOperator = pClause;
    tree->mLeft = leftTree;
    tree->mCenter = centerTree;
    leash->mTree = tree;
    leash->mClause = value;
    left->mTree = nullptr;
    left->mClause = nullptr;
    center->mTree = nullptr;
    center->mClause = nullptr;
    right->mTree = nullptr;
    right->mClause = nullptr;

    pLeashes->erase(pLeashes->indexOf(left));
    pLeashes->erase(pLeashes->indexOf(center));
    pLeashes->erase(pLeashes->indexOf(right));

    condition->mPrev->insertAfter(value);
    condition->erase();
    trueValue->erase();
    falseValue->erase();
    colon->erase();
    pClause->erase();
    return value;
}

Clause* ExpressionEvaluator::createTokenOperatorSyntaxTree(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                           Clause* pClause) {
    Clause* value = new (mClauseHeap) Clause(Clause::cType_Int, "0", "0" + 1);
    SyntaxTree* tree = new (mHeap) SyntaxTree(value);
    SyntaxLeash* leash = pLeashes->emplaceBack();

    Clause* token = pClause->mNext;
    Clause* close = token->mNext;

    SyntaxLeash* right = findSyntaxLeash(pLeashes, token);

    tree->mRight = right->mTree;
    tree->mOperator = pClause;
    tree->mLeft = nullptr;
    tree->mCenter = nullptr;
    leash->mTree = tree;
    leash->mClause = value;
    right->mTree = nullptr;
    right->mClause = nullptr;

    pLeashes->erase(pLeashes->indexOf(right));

    pClause->mPrev->insertAfter(value);
    pClause->erase();
    token->erase();
    close->erase();
    return value;
}

void ExpressionEvaluator::resolveOperatorTokenConnect(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                      Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_SingleQuote || c->mType == Clause::cType_DoubleQuote) {
            c = createTokenOperatorSyntaxTree(pLeashes, c);
        } else if (c->mType == Clause::cType_Dot) {
            const Clause* prev = c->mPrev;

            if ((prev->isWord() || prev->mType == Clause::cType_RParen ||
                 prev->mType == Clause::cType_RBracket) &&
                c->mNext->isWord()) {
                c = createBinaryOperatorSyntaxTree(pLeashes, c);
            }
        }
    }

    resolveOperatorUnary(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorUnary(sead::ObjArray<SyntaxLeash>* pLeashes,
                                               Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Plus || c->mType == Clause::cType_Minus) {
            const Clause* prev = c->mPrev;

            if (prev->mType == Clause::cType_RBracket || prev->isWord()) {
                continue;
            }
        } else if (c->mType != Clause::cType_Tilde && c->mType != Clause::cType_Not) {
            continue;
        }

        Clause* value = new (mClauseHeap) Clause(Clause::cType_Int, "0", "0" + 1);
        SyntaxTree* tree = new (mHeap) SyntaxTree(value);
        SyntaxLeash* leash = pLeashes->emplaceBack();

        SyntaxLeash* right = findSyntaxLeash(pLeashes, c->mNext);

        tree->mRight = right->mTree;
        tree->mOperator = c;
        tree->mLeft = nullptr;
        tree->mCenter = nullptr;
        leash->mTree = tree;
        leash->mClause = value;
        right->mTree = nullptr;
        right->mClause = nullptr;

        pLeashes->erase(pLeashes->indexOf(right));

        c->mPrev->insertAfter(value);
        c->mNext->erase();
        c->erase();
        c = value;
    }

    resolveOperatorMathHigh(pLeashes, pBegin, pEnd);
}

/**
 * Builds syntax trees for multiplicative operators, then resolves lower precedences.
 * @param pLeashes syntax trees of the operands
 * @param pBegin clause before the range
 * @param pEnd clause after the range
 */
void ExpressionEvaluator::resolveOperatorMathHigh(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                  Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Mul || c->mType == Clause::cType_Div || c->mType == Clause::cType_Mod) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorMathLow(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorMathLow(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                 Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Plus || c->mType == Clause::cType_Minus) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorShift(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorShift(sead::ObjArray<SyntaxLeash>* pLeashes,
                                               Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_ShiftL || c->mType == Clause::cType_ShiftR) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorCompareHigh(pLeashes, pBegin, pEnd);
}

/**
 * Builds syntax trees for relational operators, then resolves lower precedences.
 * @param pLeashes syntax trees of the operands
 * @param pBegin clause before the range
 * @param pEnd clause after the range
 */
void ExpressionEvaluator::resolveOperatorCompareHigh(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                     Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Less || c->mType == Clause::cType_Greater ||
            c->mType == Clause::cType_LessEqual || c->mType == Clause::cType_GreaterEqual) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorCompareLow(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorCompareLow(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                    Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_NotEqual || c->mType == Clause::cType_Equal) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorBitOpAnd(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorBitOpAnd(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                  Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_And) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorBitOpXor(pLeashes, pBegin, pEnd);
}

/**
 * Builds syntax trees for bitwise xor operators, then resolves lower precedences.
 * @param pLeashes syntax trees of the operands
 * @param pBegin clause before the range
 * @param pEnd clause after the range
 */
void ExpressionEvaluator::resolveOperatorBitOpXor(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                  Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Xor) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorBitOpOr(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorBitOpOr(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                 Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Or) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorLogicalAnd(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorLogicalAnd(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                    Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_LogicalAnd) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorLogicalOr(pLeashes, pBegin, pEnd);
}

void ExpressionEvaluator::resolveOperatorLogicalOr(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                   Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_LogicalOr) {
            c = createBinaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorTernary(pLeashes, pBegin, pEnd);
}

/**
 * Builds syntax trees for conditional operators.
 * @param pLeashes syntax trees of the operands
 * @param pBegin clause before the range
 * @param pEnd clause after the range
 */
void ExpressionEvaluator::resolveOperatorTernary(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                 Clause* pBegin, Clause* pEnd) {
    for (Clause* c = pBegin->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_Question) {
            c = createTernaryOperatorSyntaxTree(pLeashes, c);
        }
    }

    resolveOperatorAssignment(pLeashes, pBegin, pEnd);
}

/**
 * Assignment operators are not resolved.
 * @param pLeashes syntax trees of the operands
 * @param pBegin clause before the range
 * @param pEnd clause after the range
 */
void ExpressionEvaluator::resolveOperatorAssignment(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                    Clause* pBegin, Clause* pEnd) {}

/**
 * Builds syntax trees for all operators of a range.
 * @param pLeashes syntax trees of the operands
 * @param pBegin clause before the range
 * @param pEnd clause after the range
 */
void ExpressionEvaluator::resolveOperator(sead::ObjArray<SyntaxLeash>* pLeashes, Clause* pBegin,
                                          Clause* pEnd) {
    resolveOperatorTokenConnect(pLeashes, pBegin, pEnd);
}

Clause* ExpressionEvaluator::resolveParenthesis(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                Clause* pClose, Clause* pEnd) {
    Clause* open = pClose;

    while (open->mType != Clause::cType_LParen) {
        open = open->mPrev;
    }

    Clause* prev = open->mPrev;
    resolveOperator(pLeashes, prev, pClose);
    (*mRemoveDelegate)(open);
    (*mRemoveDelegate)(pClose);
    return prev;
}

SyntaxTree* ExpressionEvaluator::resolve(Clause* pBegin, Clause* pEnd, bool) {
    Clause* head = pBegin->mPrev;
    sead::ObjArray<SyntaxLeash> leashes;

    s32 depth = 0;
    u32 valueNum = 0;

    for (Clause* c = head->mNext; c != pEnd;) {
        Clause* next = c->mNext;
        const Clause::CharacterInfo& info = c->getInfo();

        if (info.mIsSpace) {
            (*mRemoveDelegate)(c);
        } else if (c->mType == Clause::cType_LParen) {
            depth++;
        } else if (c->mType == Clause::cType_RParen) {
            depth--;
        } else {
            bool isNotValue = c->isOperator() ||
                              (c->isSeparator() && !(c->isCloseBracket() || c->isOpenBracket()));
            valueNum += !isNotValue;
        }

        c = next;
    }

    if (depth != 0) {
        return nullptr;
    }

    leashes.allocBuffer(valueNum + 1, mHeap, -4);

    for (Clause* c = head->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_LParen || c->mType == Clause::cType_RParen) {
            continue;
        }

        const Clause::CharacterInfo& info = c->getInfo();

        if (info.mIsOperator ||
            (info.mIsSeparator && !(info.mIsCloseBracket || info.mIsOpenBracket))) {
            continue;
        }

        SyntaxLeash* leash = leashes.emplaceBack();
        leash->mTree = new (mHeap) SyntaxTree(c);
        leash->mClause = c;
    }

    for (Clause* c = head->mNext; c != pEnd; c = c->mNext) {
        if (c->mType == Clause::cType_RParen) {
            c = resolveParenthesis(&leashes, c, pEnd);
        }
    }

    resolveOperator(&leashes, head, pEnd);

    if (leashes.size() == 1) {
        SyntaxLeash* leash = leashes(0);
        SyntaxTree* tree = leash->mTree;
        leash->mClause->erase();
        leashes.freeBuffer();
        return tree;
    }

    for (s32 i = 0; i < leashes.size(); i++) {
        leashes(i)->mTree->removeClauseRecursive(mRemoveDelegate);
        delete leashes(i)->mTree;
    }

    leashes.freeBuffer();
    return nullptr;
}

}  // namespace agl::shtxt
