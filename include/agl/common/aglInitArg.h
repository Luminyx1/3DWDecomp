#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace sead {
class ArchiveRes;
class Heap;
}  // namespace sead

namespace sead::hostio {
class Node;
}  // namespace sead::hostio

namespace agl {

class InitArg {
public:
    InitArg();

    sead::Heap* mHeap = nullptr;
    u64 mWorkHeapSize = 0x800000;
    sead::Heap* mDebugHeap = nullptr;
    u64 _18 = 0x3000000;
    s32 mDynamicTextureNum = 0x100;
    u64 mDynamicTextureSize = 0x2000000;
    u64 mDynamicTextureDebugSize = 0x2000000;
    u64 mDynamicUniformBlockSize = 0x100000;
    u64 mMinGPUMemBlockSize = 0x1000;
    u32 _48 = 0x10;
    u32 _4c = 0x80;
    sead::FixedSafeString<256> mRootNodeMetaSuffix{sead::SafeString::cEmptyString};
    bool mShaderNoOption = true;
    bool mUseCheckout = false;
    bool _16a = false;
};

static_assert(sizeof(InitArg) == 0x170);

void Initialize(const InitArg& rArg);
void AppendRootNodeToOR(sead::hostio::Node* pNode);
void LoadResource(sead::ArchiveRes* pArchive);
void Finalize();

}  // namespace agl
