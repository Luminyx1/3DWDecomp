#pragma once

#include <container/seadObjArray.h>
#include <prim/seadDelegate.h>

namespace sead {
class Heap;
}  // namespace sead

namespace agl::shtxt {

class Clause;
class SyntaxTree;

class SyntaxLeash {
public:
    SyntaxTree* mTree;
    Clause* mClause;
};

class ExpressionEvaluator {
public:
    ExpressionEvaluator();
    ExpressionEvaluator(sead::Heap* pHeap, sead::Heap* pClauseHeap,
                        const sead::AnyDelegate1Const<Clause*>* pRemoveDelegate);

    void initialize(sead::Heap* pHeap, sead::Heap* pClauseHeap,
                    const sead::AnyDelegate1Const<Clause*>* pRemoveDelegate);
    SyntaxLeash* findSyntaxLeash(sead::ObjArray<SyntaxLeash>* pLeashes,
                                 const Clause* pClause) const;
    Clause* createBinaryOperatorSyntaxTree(sead::ObjArray<SyntaxLeash>* pLeashes,
                                               Clause* pClause);
    Clause* createTernaryOperatorSyntaxTree(sead::ObjArray<SyntaxLeash>* pLeashes,
                                                Clause* pClause);
    Clause* createTokenOperatorSyntaxTree(sead::ObjArray<SyntaxLeash>* pLeashes,
                                              Clause* pClause);
    void resolveOperatorTokenConnect(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorUnary(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorMathHigh(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorMathLow(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorShift(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorCompareHigh(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorCompareLow(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorBitOpAnd(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorBitOpXor(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorBitOpOr(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorLogicalAnd(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorLogicalOr(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorTernary(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperatorAssignment(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    void resolveOperator(sead::ObjArray<SyntaxLeash>*, Clause*, Clause*);
    Clause* resolveParenthesis(sead::ObjArray<SyntaxLeash>* pLeashes, Clause* pClose, Clause* pEnd);
    SyntaxTree* resolve(Clause* pBegin, Clause* pEnd, bool);

private:
    sead::Heap* mHeap = nullptr;
    sead::Heap* mClauseHeap = nullptr;
    const sead::AnyDelegate1Const<Clause*>* mRemoveDelegate = nullptr;
};

}  // namespace agl::shtxt
