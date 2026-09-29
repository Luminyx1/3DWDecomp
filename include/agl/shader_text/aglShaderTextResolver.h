#pragma once

#include <prim/seadBitFlag.h>
#include <prim/seadDelegate.h>
#include <prim/seadSafeString.h>
#include "shader_text/aglShaderTextDefineLinker.h"
#include "shader_text/aglShaderTextExpressionEvaluator.h"

namespace sead {
class Heap;
}

namespace agl::shtxt {

class Clause;
class SyntaxTree;

struct MacroReplaceInfo {
    DefineLinker mLinker;
    bool mIsValid;
};
static_assert(sizeof(MacroReplaceInfo) == 0x50);

struct MacroDeployInfo {
    sead::FixedSafeString<64> mName;
    u32 mLength;
    bool mIsActive;
};
static_assert(sizeof(MacroDeployInfo) == 0x60);

class Resolver {
public:
    struct DefineStackInfo {
        DefineLinker* mRoot;
        DefineLinker* mTable[256];
    };

    Resolver();
    ~Resolver();

    void removeClause(Clause* pClause) const;
    void removeClause(Clause* pBegin, Clause* pEnd, bool includeEnd) const;
    void restructDefineStack(DefineStackInfo* pInfo);
    void removeDefineStack(DefineStackInfo* pInfo, u32 depth);
    void pushDefineStack();
    void popDefineStack();
    DefineLinker* createDefineLinker(Clause* pBegin, Clause* pEnd, bool* pIsValid);
    const DefineLinker* compareMacro(const Clause* pClause, u32 hash) const;
    u32 calcMacroNum() const;
    const DefineLinker* compareMacro(const Clause* pClause) const;
    Clause* replaceMacro(Clause* pClause, const DefineLinker* pLinker, bool isConcat) const;
    Clause* resolveMacro(Clause* pClause, bool isExpandDefined, bool isConcat) const;
    SyntaxTree* resolveExpression(Clause* pBegin, Clause* pEnd, bool isStatic);
    Clause* resolvePreprocessExpression(bool* pIsStatic, bool* pResult, Clause* pSharp,
                                        bool isStatic);
    bool resolvePreprocessBlock(Clause* pSharp, bool isStatic);
    bool markPreprocessBlock(Clause** ppBlockBegin, Clause** ppBlockEnd, Clause** ppPartEnd,
                             Clause* pSharp, bool isStatic);
    bool resolveProgramBlock(Clause* pIf, bool* pResult);
    bool resolveTernaryBlock(Clause* pClause);
    void initialize(sead::Heap* pHeap, sead::Heap* pUnitHeap, sead::Heap* pFrameHeap,
                    const char* pText, Clause* pRoot, DefineLinker* pDefineRoot,
                    sead::AnyDelegate1Const<Clause*>* pRemoveDelegate);
    void setMacroReplaceInfo(const MacroReplaceInfo* pInfos, u32 num);
    void setMacroDeployInfo(const MacroDeployInfo* pInfos, u32 num);
    void resolveStaticBranch(Clause* pBegin, Clause* pEnd);
    void executePart(Clause* pSharp, Clause* pEnd, Clause* pSkip);
    void executeImpl(Clause* pBegin, Clause* pEnd, bool isInner);
    bool execute(bool isResolvePreprocess, bool isResolveMacro, bool isInner, bool isStaticMacro,
                 bool isKeepUnresolved, bool isResolveStaticBranch, bool isStaticExpression);

private:
    sead::Heap* mHeap = nullptr;
    sead::Heap* mUnitHeap = nullptr;
    sead::Heap* mFrameHeap = nullptr;
    const char* mText = nullptr;
    Clause* mRoot = nullptr;
    u32 mDefineStackDepth = 0;
    DefineStackInfo mDefineStack = {};
    const MacroReplaceInfo* mReplaceInfos = nullptr;
    u32 mReplaceInfoNum = 0;
    MacroDeployInfo* mDeployInfos = nullptr;
    u32 mDeployInfoNum = 0;
    ExpressionEvaluator mEvaluator;
    const sead::AnyDelegate1Const<Clause*>* mRemoveDelegate = nullptr;
    sead::SafeString mErrorMessage;
    sead::BitFlag32 mFlags;
};
static_assert(sizeof(Resolver) == 0x890);

}  // namespace agl::shtxt
