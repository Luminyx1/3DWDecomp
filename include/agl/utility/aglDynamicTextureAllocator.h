#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <mc/seadCoreInfo.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

#include "common/aglDisplayList.h"
#include "common/aglGPUMemAddr.h"
#include "common/aglTextureData.h"
#include "common/aglTextureEnum.h"
#include "utility/aglTextureMemoryAllocator.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl {
class DrawContext;
}  // namespace agl

namespace agl::utl {

class TextureDataEx;


class DynamicTextureAllocator : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(DynamicTextureAllocator)

    DynamicTextureAllocator();
    virtual ~DynamicTextureAllocator();

public:
    enum AllocateType {
        cAllocateType_0 = 0,
        cAllocateType_1 = 1,
        cAllocateType_2 = 2,
        cAllocateType_3 = 3,
    };

    enum ContextFlag {
        cContextFlag_Cache = 1 << 0,
        cContextFlag_DisplayList = 1 << 1,
    };

    struct Context {
        sead::Buffer<TextureMemoryAllocator> mAllocators;
        DisplayList mDisplayList;
        void* mSuspendMemory;
        u32 mSuspendSize;
        sead::BitFlag32 mFlags;
        u8 mIndex;
    };
    static_assert(sizeof(Context) == 0x288);

    struct Storage {
        ~Storage() {}

        GPUMemVoidAddr mAddr;
        u64 mSize;
        bool mIsUseStorage;
        sead::Buffer<TextureMemoryAllocator::Storage> mStorage;
    };
    static_assert(sizeof(Storage) == 0x38);

    struct FreeAddr : GPUMemAddr<u8> {
        ~FreeAddr() {}

        FreeAddr& operator=(const GPUMemAddr<u8>& rAddr)
        {
            GPUMemAddr<u8>::operator=(rAddr);
            return *this;
        }
    };

    void initialize(s32 textureNum, u64 size, u64 debugSize, sead::Heap* pHeap,
                    sead::Heap* pDebugHeap);
    void setDebugHeap(sead::Heap* pHeap);
    void calc();

    TextureData* alloc(DrawContext* pDrawContext, const sead::SafeString& rName,
                       TextureFormat format, u32 width, u32 height, u32 mipLevelNum,
                       GPUMemVoidAddr* pAddr, AllocateType type, bool b1, bool b2);
    TextureData* allocArray(DrawContext* pDrawContext, const sead::SafeString& rName,
                            TextureFormat format, u32 width, u32 height, u32 arrayNum,
                            u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                            bool b2);
    TextureData* alloc3D(DrawContext* pDrawContext, const sead::SafeString& rName,
                         TextureFormat format, u32 width, u32 height, u32 depth,
                         u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                         bool b2);
    TextureData* allocCube(DrawContext* pDrawContext, const sead::SafeString& rName,
                           TextureFormat format, u32 width, u32 mipLevelNum,
                           GPUMemVoidAddr* pAddr, AllocateType type, bool b1, bool b2);
    TextureData* allocCubeArray(DrawContext* pDrawContext, const sead::SafeString& rName,
                                TextureFormat format, u32 width, u32 arrayNum, u32 mipLevelNum,
                                GPUMemVoidAddr* pAddr, AllocateType type, bool b1, bool b2);
    TextureData* allocMultiSample(DrawContext* pDrawContext, const sead::SafeString& rName,
                                  TextureFormat format, u32 width, u32 height,
                                  MultiSampleType multiSample, GPUMemVoidAddr* pAddr,
                                  AllocateType type, bool b1, bool b2);
    TextureData* allocWithoutContext(DrawContext* pDrawContext, const sead::SafeString& rName,
                                     TextureFormat format, u32 width, u32 height,
                                     u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type,
                                     bool b1, bool b2);
    TextureData* allocWithoutContext(DrawContext* pDrawContext, const sead::SafeString& rName,
                                     const TextureData& rTextureData, GPUMemVoidAddr* pAddr,
                                     AllocateType type, bool b1, bool b2);
    TextureData* allocArrayWithoutContext(DrawContext* pDrawContext, const sead::SafeString& rName,
                                          TextureFormat format, u32 width, u32 height,
                                          u32 arrayNum, u32 mipLevelNum, GPUMemVoidAddr* pAddr,
                                          AllocateType type, bool b1, bool b2);
    TextureData* alloc3DWithoutContext(DrawContext* pDrawContext, const sead::SafeString& rName,
                                       TextureFormat format, u32 width, u32 height, u32 depth,
                                       u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type,
                                       bool b1, bool b2);
    TextureData* allocCubeWithoutContext(DrawContext* pDrawContext, const sead::SafeString& rName,
                                         TextureFormat format, u32 width, u32 mipLevelNum,
                                         GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                                         bool b2);
    TextureData* allocCubeArrayWithoutContext(DrawContext* pDrawContext,
                                              const sead::SafeString& rName,
                                              TextureFormat format, u32 width, u32 arrayNum,
                                              u32 mipLevelNum, GPUMemVoidAddr* pAddr,
                                              AllocateType type, bool b1, bool b2);
    TextureData* allocMultiSampleWithoutContext(DrawContext* pDrawContext,
                                                const sead::SafeString& rName,
                                                TextureFormat format, u32 width, u32 height,
                                                MultiSampleType multiSample,
                                                GPUMemVoidAddr* pAddr, AllocateType type,
                                                bool b1, bool b2);
    bool free(const TextureData* pTexture);

    void beginCache();
    void endCache();
    bool queryTextureMemoryInfo(const TextureMemoryAllocator::MemoryBlock* pBlock,
                                const char** ppName, s32* pContextIndex) const;
    void dumpAll() const;
    u64 getUseSize() const;
    u64 getUseSizeWithoutContext() const;
    void dump() const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    Context& getCurrentContext() { return mContexts[sead::CoreInfo::getCurrentCoreId()]; }
    bool isCacheEnabled()
    {
        const auto& rFlags =
            *reinterpret_cast<const volatile u32*>(&getCurrentContext().mFlags);
        return (rFlags & cContextFlag_Cache) != 0;
    }

private:
    TextureDataEx* alloc_(DrawContext* pDrawContext, Context* pContext,
                          const sead::SafeString& rName, TextureFormat format, TextureType type,
                          u32 width, u32 height, u32 mipLevelNum, MultiSampleType multiSample,
                          u32 slice, GPUMemVoidAddr* pAddr, bool withContext, AllocateType type_,
                          bool b1, bool b2);
    bool free_(const TextureData* pTexture);
    bool isValid_(const Context* pContext) const;
    bool isContextValid_(const Context* pContext) const;
    void dump_(s32 index) const;

    static constexpr s32 cContextNum = 4;
    static constexpr s32 cStorageNum = 3;

    sead::BitFlag32 mFlags;
    Context mContexts[cContextNum];
    sead::Buffer<TextureDataEx> mTextures;
    sead::CriticalSection mTextureCS;
    sead::CriticalSection mAllocatorCS;
    s32 mCurrentContext = 0;
    u64 mSize = 0;
    sead::Heap* mDebugHeap = nullptr;
    sead::Buffer<Storage> mStorages;
    u32 mSearchIndex = 0;
    u16 mFrame = 0;
    sead::Buffer<FreeAddr> mFreeAddrs;
    u16 mFreeAddrNum = 0;
};
static_assert(sizeof(DynamicTextureAllocator) == 0xb28);

class TextureDataEx : public TextureData {
    friend class DynamicTextureAllocator;

public:
    TextureDataEx();

    bool isSame(const sead::SafeString& rName, TextureFormat format, TextureType type, u32 width,
                u32 height, MultiSampleType multiSample, u32 slice, s32 mipLevelNum,
                u32 flags) const;
    void release();
    void initialize(TextureFormat format, TextureType type, u32 width, u32 height,
                    u32 mipLevelNum, MultiSampleType multiSample, u32 slice,
                    GPUMemVoidAddr* pAddr, u32 flags);
    void reset(DrawContext* pDrawContext, const sead::SafeString& rName, bool updateImage,
               u32 frame);
    void genMessage(sead::hostio::Context* pContext);
    void clear();

    TextureMemoryAllocator& getAllocator() const { return mContext->mAllocators[mAllocatorIndex]; }

private:
    TextureMemoryAllocator::MemoryBlock* mMemoryBlock;
    TextureMemoryAllocator::AllocateArg mAllocateArg;
    DynamicTextureAllocator::Context* mContext;
    sead::SafeString mName;
    u32 mUseSize;
    GPUMemVoidAddr mUserAddr;
    u8 mMagic;
    u8 mAllocateType;
    u8 mAllocatorIndex;
    u8 mMipLevelNum;
    sead::BitFlag8 mOption;
    u8 _18d;
    u16 mFrame;
    u8 mAttributeFlags;
    sead::BitFlag8 mState;
    sead::BitFlag8 mInfo;
    sead::FixedSafeString<32> mLabel;
};
static_assert(sizeof(TextureDataEx) == 0x1d0);

}  // namespace agl::utl
