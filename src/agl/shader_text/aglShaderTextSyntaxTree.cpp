#include "shader_text/aglShaderTextSyntaxTree.h"

#include <basis/seadNew.h>
#include <heap/seadHeap.h>
#include <prim/seadMemUtil.h>
#include <prim/seadSafeString.h>

#include "shader_text/aglShaderTextClause.h"

namespace agl::shtxt {

namespace {

Clause* cloneClause(sead::Heap* pHeap, const Clause* pSrc) {
    Clause* clause = new (pHeap) Clause(pSrc->getType(), pSrc->mBegin, pSrc->mEnd);
    clause->mFlag = pSrc->mFlag;
    return clause;
}

}  // namespace

/**
 * Constructs a value node of a syntax tree.
 * @param pValue value clause of the node
 */
SyntaxTree::SyntaxTree(Clause* pValue) : mValue(pValue) {}

/**
 * Destroys the node and its children.
 */
SyntaxTree::~SyntaxTree() {
    delete mLeft;
    delete mCenter;
    delete mRight;
}

/**
 * Removes the clauses of the node and its children.
 * @param pRemoveDelegate delegate that removes a clause
 */
void SyntaxTree::removeClauseRecursive(const sead::AnyDelegate1Const<Clause*>* pRemoveDelegate) {
    if (mLeft != nullptr) {
        mLeft->removeClauseRecursive(pRemoveDelegate);
    }

    if (mCenter != nullptr) {
        mCenter->removeClauseRecursive(pRemoveDelegate);
    }

    if (mRight != nullptr) {
        mRight->removeClauseRecursive(pRemoveDelegate);
    }

    if (mOperator != nullptr) {
        (*pRemoveDelegate)(mOperator);
        mOperator = nullptr;
    }

    if (mValue != nullptr) {
        (*pRemoveDelegate)(mValue);
        mValue = nullptr;
    }
}

/**
 * Gets the numeric value of a value node.
 * @return the value
 */
f64 SyntaxTree::checkAndGetValue() const {
    if (mValue->mType == Clause::cType_Word) {
        mIsValid = false;
    }

    return mValue->forceNumber();
}

/**
 * Evaluates a child node, or the value of this node if there is no child.
 * @param pTree child node, may be nullptr
 * @return the value
 */
f64 SyntaxTree::checkAndEvaluate(const SyntaxTree* pTree) const {
    if (pTree != nullptr) {
        f64 value = pTree->evaluate();

        if (!pTree->mIsValid) {
            mIsValid = false;
        }

        return value;
    }

    return checkAndGetValue();
}

/**
 * Evaluates the expression of the node.
 * @return the value
 */
f64 SyntaxTree::evaluate() const {
    mIsValid = true;

    if (mOperator == nullptr) {
        mResult = checkAndGetValue();
        return mResult;
    }

    u8 type = mOperator->mType;

    if (type == Clause::cType_SingleQuote || type == Clause::cType_DoubleQuote) {
        mIsValid = true;
        mResult = 1.0;
        return 1.0;
    }

    if (type == Clause::cType_Dot) {
        mIsValid = false;
        mResult = 0.0;
        return 0.0;
    }

    f64 left = checkAndEvaluate(mLeft);
    f64 center = checkAndEvaluate(mCenter);
    f64 right = checkAndEvaluate(mRight);

    switch (mOperator->mType) {
    case Clause::cType_Plus:
    case Clause::cType_PlusAssign:
        mResult = left + right;
        return mResult;
    case Clause::cType_Minus:
    case Clause::cType_MinusAssign:
        mResult = left - right;
        return mResult;
    case Clause::cType_Mul:
    case Clause::cType_MulAssign:
        mResult = left * right;
        return mResult;
    case Clause::cType_Div:
    case Clause::cType_DivAssign:
        mResult = left / right;
        return mResult;
    case Clause::cType_Mod:
    case Clause::cType_ModAssign:
        mResult = static_cast<u64>(left) % static_cast<u64>(right);
        return mResult;
    case Clause::cType_Assign:
        mResult = right;
        return mResult;
    case Clause::cType_Tilde:
        mResult = ~static_cast<u64>(right);
        return mResult;
    case Clause::cType_And:
    case Clause::cType_AndAssign:
        mResult = static_cast<u64>(left) & static_cast<u64>(right);
        return mResult;
    case Clause::cType_Or:
    case Clause::cType_OrAssign:
        mResult = static_cast<u64>(left) | static_cast<u64>(right);
        return mResult;
    case Clause::cType_Xor:
    case Clause::cType_XorAssign:
        mResult = static_cast<u64>(left) ^ static_cast<u64>(right);
        return mResult;
    case Clause::cType_ShiftL:
    case Clause::cType_ShiftLAssign:
        mResult = static_cast<u64>(left) << static_cast<u64>(right);
        return mResult;
    case Clause::cType_ShiftR:
    case Clause::cType_ShiftRAssign:
        mResult = static_cast<u64>(left) >> static_cast<u64>(right);
        return mResult;
    case Clause::cType_Less:
        mResult = left < right ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_Greater:
        mResult = left > right ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_LogicalAnd:
        if (mIsValid) {
            mResult = left != 0.0 && right != 0.0;
            return mResult;
        }

        if (left == 0.0 && mLeft->mIsValid) {
            mResult = 0.0;
            mIsValid = true;
            return 0.0;
        }

        {
            bool isRightValid = mRight->mIsValid;
            mResult = 0.0;

            if (right == 0.0 && isRightValid) {
                mIsValid = true;
                return 0.0;
            }
        }

        mIsValid = false;
        return 0.0;
    case Clause::cType_LogicalOr:
        if (mIsValid) {
            mResult = left != 0.0 || right != 0.0;
            return mResult;
        }

        if (left == 1.0 && mLeft->mIsValid) {
            mResult = 1.0;
            mIsValid = true;
            return 1.0;
        }

        if (right == 1.0 && mRight->mIsValid) {
            mResult = 1.0;
            mIsValid = true;
            return 1.0;
        }

        mResult = 0.0;
        mIsValid = false;
        return 0.0;
    case Clause::cType_LessEqual:
        mResult = left <= right ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_GreaterEqual:
        mResult = left >= right ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_NotEqual:
        mResult = left != right ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_Equal:
        mResult = left == right ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_Not:
        mResult = right == 0.0 ? 1.0 : 0.0;
        return mResult;
    case Clause::cType_Question:
        mResult = static_cast<u64>(left) != 0 ? center : right;
        return mResult;
    default:
        return mResult;
    }
}

/**
 * Creates the clauses of the node and its children, replacing evaluated parts with their values.
 * @param pHeap heap for the clauses
 * @param pStringHeap heap for the value strings
 * @return first clause of the created list
 */
Clause* SyntaxTree::constructRecursive(sead::Heap* pHeap, sead::Heap* pStringHeap) const {
    if (mOperator == nullptr) {
        const Clause* value = mValue;

        if (value->mFlag & 1) {
            if (mResult == 1.0) {
                return new (pHeap) Clause(Clause::cType_Int, "true", "true" + 4);
            }

            if (mResult == 0.0) {
                return new (pHeap) Clause(Clause::cType_Int, "false", "false" + 5);
            }
        }

        return cloneClause(pHeap, value);
    }

    if (mIsValid) {
        switch (mOperator->mType) {
        case Clause::cType_Mod:
        case Clause::cType_And:
        case Clause::cType_Or:
        case Clause::cType_Xor:
        case Clause::cType_ShiftL:
        case Clause::cType_ShiftR:
        case Clause::cType_ModAssign:
        case Clause::cType_AndAssign:
        case Clause::cType_OrAssign:
        case Clause::cType_XorAssign:
        case Clause::cType_ShiftLAssign:
        case Clause::cType_ShiftRAssign: {
            sead::FixedSafeString<32> text;
            u32 length = text.format("%d", static_cast<s64>(mResult));
            char* str = static_cast<char*>(pStringHeap->alloc(length, 8));
            sead::MemUtil::copy(str, text.cstr(), length);
            return new (pHeap) Clause(Clause::cType_Real, str, str + length);
        }
        case Clause::cType_Plus:
        case Clause::cType_Minus:
        case Clause::cType_Mul:
        case Clause::cType_Div:
        case Clause::cType_PlusAssign:
        case Clause::cType_MinusAssign:
        case Clause::cType_MulAssign:
        case Clause::cType_DivAssign:
        case Clause::cType_Question: {
            sead::FixedSafeString<32> text;
            u32 length = text.format("%f", mResult);
            char* str = static_cast<char*>(pStringHeap->alloc(length, 8));
            sead::MemUtil::copy(str, text.cstr(), length);
            return new (pHeap) Clause(Clause::cType_Real, str, str + length);
        }
        case Clause::cType_Assign:
        case Clause::cType_Tilde:
        case Clause::cType_Less:
        case Clause::cType_Greater:
        case Clause::cType_LogicalAnd:
        case Clause::cType_LogicalOr:
        case Clause::cType_LessEqual:
        case Clause::cType_GreaterEqual:
        case Clause::cType_NotEqual:
        case Clause::cType_Equal:
        case Clause::cType_Not:
            if (mResult == 1.0) {
                return new (pHeap) Clause(Clause::cType_Int, "true", "true" + 4);
            }

            if (mResult == 0.0) {
                return new (pHeap) Clause(Clause::cType_Int, "false", "false" + 5);
            }

            break;
        default:
            break;
        }
    }

    u32 type = mOperator->mType;
    const Clause::CharacterInfo& info = Clause::cCharacterTable[type];

    if (info.mIsUnaryOperator) {
        Clause* list = new (pHeap) Clause(Clause::cType_LParen, "(", "(" + 1);
        list->mPrev->insertListAfter(cloneClause(pHeap, mOperator));
        list->mPrev->insertListAfter(mRight->constructRecursive(pHeap, pStringHeap));
        list->mPrev->insertListAfter(new (pHeap) Clause(Clause::cType_RParen, ")", ")" + 1));
        return list;
    }

    if (type - Clause::cType_SingleQuote < 2) {
        Clause* list = new (pHeap) Clause(Clause::cType_LParen, "(", "(" + 1);
        list->mPrev->insertListAfter(cloneClause(pHeap, mOperator));
        list->mPrev->insertListAfter(mRight->constructRecursive(pHeap, pStringHeap));
        list->mPrev->insertListAfter(cloneClause(pHeap, mOperator));
        list->mPrev->insertListAfter(new (pHeap) Clause(Clause::cType_RParen, ")", ")" + 1));
        return list;
    }

    if (info.mIsBinaryOperator) {
        if ((type & ~1u) == Clause::cType_LogicalAnd) {
            Clause* list = nullptr;

            if (mLeft->mIsValid) {
                list = mRight->constructRecursive(pHeap, pStringHeap);
            } else if (mRight->mIsValid) {
                list = mLeft->constructRecursive(pHeap, pStringHeap);
            }

            if (list != nullptr) {
                return list;
            }
        }
    } else if (type != Clause::cType_Dot) {
        if (type != Clause::cType_Question) {
            return nullptr;
        }

        Clause* list = new (pHeap) Clause(Clause::cType_LParen, "(", "(" + 1);
        list->mPrev->insertListAfter(mLeft->constructRecursive(pHeap, pStringHeap));
        list->mPrev->insertListAfter(cloneClause(pHeap, mOperator));
        list->mPrev->insertListAfter(mCenter->constructRecursive(pHeap, pStringHeap));
        list->mPrev->insertListAfter(new (pHeap) Clause(Clause::cType_Colon, ":", ":" + 1));
        list->mPrev->insertListAfter(mRight->constructRecursive(pHeap, pStringHeap));
        list->mPrev->insertListAfter(new (pHeap) Clause(Clause::cType_RParen, ")", ")" + 1));
        return list;
    }

    Clause* list = new (pHeap) Clause(Clause::cType_LParen, "(", "(" + 1);
    list->mPrev->insertListAfter(mLeft->constructRecursive(pHeap, pStringHeap));
    list->mPrev->insertListAfter(cloneClause(pHeap, mOperator));
    list->mPrev->insertListAfter(mRight->constructRecursive(pHeap, pStringHeap));
    list->mPrev->insertListAfter(new (pHeap) Clause(Clause::cType_RParen, ")", ")" + 1));
    return list;
}

/**
 * Creates the clauses of the tree enclosed in parentheses.
 * @param pHeap heap for the clauses
 * @param pStringHeap heap for the value strings
 * @return first clause of the created list
 */
Clause* SyntaxTree::construct(sead::Heap* pHeap, sead::Heap* pStringHeap) const {
    Clause* list = new (pHeap) Clause(Clause::cType_LParen, "(", "(" + 1);
    list->mPrev->insertListAfter(constructRecursive(pHeap, pStringHeap));
    list->mPrev->insertListAfter(new (pHeap) Clause(Clause::cType_RParen, ")", ")" + 1));
    return list;
}

}  // namespace agl::shtxt
