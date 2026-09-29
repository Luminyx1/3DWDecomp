#pragma once

#include <prim/seadSafeString.h>

namespace sead {
class Heap;
}  // namespace sead

namespace agl::shtxt {

class Clause;
class DefineLinker;
struct MacroDeployInfo;
struct MacroReplaceInfo;

class Preprocessor {
public:
    enum Flag {
        cFlag_ResolveMacro = 1 << 0,
        cFlag_ResolveExpression = 1 << 1,
        cFlag_ResolveBranch = 1 << 2,
        cFlag_ResolveDefine = 1 << 3,
        cFlag_ResolveTernary = 1 << 4,
        cFlag_ResolveProgram = 1 << 5,
        cFlag_RemoveComment = 1 << 6,
        cFlag_ForceLF = 1 << 7,
        cFlag_ReduceSpace = 1 << 8,
        cFlag_ReduceLF = 1 << 9,
        cFlag_Format = 1 << 10,
        cFlag_FormatWithTab = 1 << 11,
    };

    Preprocessor(sead::Heap* pHeap, sead::Heap* pWorkHeap);
    ~Preprocessor();

    void finalize();
    void initialize(const char* pText);
    void removeClause(Clause* pClause) const;
    void removeClause(Clause* pBegin, Clause* pEnd, bool includeEnd) const;
    void removeClauseAll();
    void removeDefineLinkerAll();
    void setReplacedMacro(const char** pNames, const char** pValues, u32 num);
    void setDeployMacro(const char** pNames, u32 num);
    void appendMacro(const DefineLinker* pLinker);
    bool preprocess(u32 flags, u64 clauseNum, u64 appendHeapSize);
    void removeComment();
    void forceLF();
    void reduceSpace();
    void reduceLF();
    void format(bool useTab);
    s32 construct(sead::BufferedSafeString* pDst) const;
    u64 calcConstructLength() const;

    const sead::SafeString& getErrorMessage() const { return mErrorMessage; }

private:
    sead::Heap* mHeap;
    sead::Heap* mWorkHeap;
    sead::Heap* mLexicalHeap = nullptr;
    sead::Heap* mSyntacticHeap = nullptr;
    sead::Heap* mFormatHeap = nullptr;
    Clause* mRoot;
    DefineLinker* mDefineRoot;
    const char* mText = nullptr;
    MacroReplaceInfo* mReplaceInfos = nullptr;
    u32 mReplaceInfoNum = 0;
    MacroDeployInfo* mDeployInfos = nullptr;
    u32 mDeployInfoNum = 0;
    DefineLinker* mAppendedMacros = nullptr;
    sead::SafeString mErrorMessage;
};
static_assert(sizeof(Preprocessor) == 0x78);

}  // namespace agl::shtxt
