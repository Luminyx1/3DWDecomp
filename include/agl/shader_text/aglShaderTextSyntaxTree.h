#pragma once

#include <basis/seadTypes.h>
#include <prim/seadDelegate.h>

namespace sead {
class Heap;
}  // namespace sead

namespace agl::shtxt {

class Clause;

class SyntaxTree {
public:
    explicit SyntaxTree(Clause* pValue);
    ~SyntaxTree();

    void removeClauseRecursive(const sead::AnyDelegate1Const<Clause*>* pRemoveDelegate);
    f64 checkAndGetValue() const;
    f64 checkAndEvaluate(const SyntaxTree* pTree) const;
    f64 evaluate() const;
    Clause* constructRecursive(sead::Heap* pHeap, sead::Heap* pStringHeap) const;
    Clause* construct(sead::Heap* pHeap, sead::Heap* pStringHeap) const;

    SyntaxTree* mLeft = nullptr;
    SyntaxTree* mCenter = nullptr;
    SyntaxTree* mRight = nullptr;
    Clause* mOperator = nullptr;
    Clause* mValue;
    mutable f64 mResult = 0.0;
    mutable bool mIsValid = false;
};
static_assert(sizeof(SyntaxTree) == 0x38);

}  // namespace agl::shtxt
