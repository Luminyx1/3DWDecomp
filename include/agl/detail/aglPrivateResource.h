#pragma once

#include <basis/seadTypes.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>
#include "common/aglGPUMemBlock.h"

namespace sead {
class ArchiveFileDevice;
class ArchiveRes;
}  // namespace sead

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl {
class TextureSampler;
}  // namespace agl

namespace agl::detail {

class PrivateResource : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(PrivateResource)

    PrivateResource();
    virtual ~PrivateResource();

public:
    struct LockedCacheMemory {
        void* mpBuffer;
        u32 mSize;
    };

    void initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap, u64 workHeapSize, u64 unused);
    void createArchive(sead::ArchiveRes* pArchive);
    const void* getFileFromArc(const sead::SafeString& rPath);
    void setLockedCacheMemory(u32 index, void* pBuffer, u32 size);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    sead::Heap* getWorkHeap() const { return mWorkHeap; }
    sead::Heap* getDebugHeap() const { return mDebugHeap; }
    sead::Heap* getShaderTextHeap() const { return static_cast<sead::Heap*>(_30); }
    sead::BufferedSafeString* getShaderSourceBuffer(s32 type) const
    {
        return reinterpret_cast<sead::BufferedSafeString* const*>(_38)[type];
    }
    char* getWorkBuffer() const { return static_cast<char*>(_60); }
    s32 getWorkBufferSize() const { return _58; }
    const TextureSampler* getCursorTextureSampler() const { return mCursorTextureSampler; }

private:
    sead::Heap* mWorkHeap = nullptr;
    void* _30 = nullptr;
    u8 _38[0x58 - 0x38];
    u32 _58 = 0;
    void* _60 = nullptr;
    sead::ArchiveFileDevice* mArchiveFileDevice = nullptr;
    sead::ArchiveRes* mArchive = nullptr;
    sead::Heap* mDebugHeap = nullptr;
    u8* mCursorTextureSamplerBuffer = nullptr;
    TextureSampler* mCursorTextureSampler = nullptr;
    GPUMemBlock<u8> mCursorTextureMemory;
    void (*mDebugPrintFn)(const sead::SafeString& rString);
    LockedCacheMemory mLockedCacheMemory[3];
};

static_assert(sizeof(PrivateResource) == 0x100);

}  // namespace agl::detail
