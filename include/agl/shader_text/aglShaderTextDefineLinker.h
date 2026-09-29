#pragma once

#include <basis/seadTypes.h>
#include "shader_text/aglShaderTextClause.h"

namespace sead {
class Heap;
}

namespace agl::shtxt {

class DefineLinker : public helper::Knot<DefineLinker> {
public:
    DefineLinker();
    ~DefineLinker();

    void clear();
    bool set(sead::Heap* pHeap, const Clause* pBegin, const Clause* pEnd);
    bool setImpl(sead::Heap* pHeap, const Clause* pBegin, const Clause* pEnd, bool calcHash);
    bool setDirect(Clause* pRoot, u32 index, bool calcHash);
    void updateHash();
    void replace(sead::Heap* pHeap, const Clause* pBegin, const Clause* pEnd, bool includeEnd);
    DefineLinker* clone(sead::Heap* pHeap, sead::Heap* pClauseHeap) const;
    DefineLinker* cloneAll(sead::Heap* pHeap, sead::Heap* pClauseHeap) const;

    Clause* mRoot = nullptr;
    Clause* mName = nullptr;
    Clause* mArgEnd = nullptr;
    Clause* mValueBegin = nullptr;
    Clause* mValueEnd = nullptr;
    u64 _38 = 0;
    u32 mHash = 0xffffffff;
    u16 mDepth = 0xffff;
    bool mIsEnabled = true;
    bool mIsResolvable = true;
};
static_assert(sizeof(DefineLinker) == 0x48);

}  // namespace agl::shtxt
