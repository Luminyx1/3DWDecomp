#pragma once

#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadDelegate.h>
#include <prim/seadSafeString.h>
#include <thread/seadAtomic.h>
#include <thread/seadCriticalSection.h>

#include "common/aglDisplayList.h"
#include "common/aglGPUCommon.hpp"
#include "common/aglShaderEnum.h"
#include "common/aglTextureEnum.h"
#include "driver/aglGraphicsDriverMgr.h"

namespace sead {
class Color4f;
}

namespace agl {
class ShaderLocation;
class TextureData;
}  // namespace agl

namespace agl::driver {

class NVNMgr : public GraphicsDriverMgr {
    friend class agl::DrawContext;
    friend class agl::DisplayList;

public:
    struct SamplerKey {
        SamplerKey() : mFlags(0) {}

        u64 mSampler[0x60 / sizeof(u64)];
        u16 mRefCount;
        u16 mKey;
        u8 mFlags;
    };
    static_assert(sizeof(SamplerKey) == 0x68);

    struct TextureInfo {
        sead::Atomic<s32> mRefCount;
        const char* mName;
        sead::Atomic<u32> mFlags;
    };
    static_assert(sizeof(TextureInfo) == 0x18);

    enum Barrier {
        cBarrier_Texture = 0,
        cBarrier_Shader = 1,
    };

    static NVNMgr* createInstance(sead::Heap* pHeap);
    static NVNMgr* instance() { return static_cast<NVNMgr*>(GraphicsDriverMgr::instance()); }

    NVNMgr();
    ~NVNMgr() override;

    void waitDrawDone(DrawContext* pDrawContext) const override;

    void initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap);
    bool registerFastClearColor(TextureFormat format, const sead::Color4f& rColor);
    bool registerFastClearColor(TextureFormat format, const sead::Vector4<u32>& rColor);
    bool registerFastClearColor(TextureFormat format, const sead::Vector4<s32>& rColor);
    bool registerFastClearDepth(f32 depth);

    u32 registerSampler(const NVNsampler* pSampler, const char* pName);
    bool countupSampler(u32 id);
    bool releaseSampler(u32 id);
    bool isEqual(u32 id, const NVNsampler& rSampler) const;
    static bool isEqual(const NVNsampler& rA, const NVNsampler& rB);

    s32 registerTexture(const NVNtexture* pTexture, const NVNtextureView* pView,
                        const char* pName);
    bool countupTexture(u32 id);
    bool releaseTexture(u32 id);
    static bool isEqual(const NVNtexture& rA, const NVNtexture& rB);

    static NVNshaderStage getNVNshaderStage(ShaderType type);

    NVNdevice* getNvnDevice() const { return mDevice; }
    NVNqueue* getNvnQueue() const { return mQueue; }
    u32 getTextureFlags(bool compressible, bool renderTarget, NVNformat format) const;
    void setMemoryPoolSettingTexture(sead::BitFlag32* pFlags) const;

    void enableTiledCaching(DrawContext* pDrawContext, u32 tileWidth, u32 tileHeight) const;
    void disableTiledCaching(DrawContext* pDrawContext) const;
    void beginTiledCachingDebug(DrawContext* pDrawContext) const;
    void endTiledCachingDebug(DrawContext* pDrawContext) const;

    void countDown(s32 index);
    void dampRegisteredTextureList() const;

    void nvnMemoryPoolInitialize(NVNmemoryPool* pPool, const NVNmemoryPoolBuilder* pBuilder);
    void nvnCommandBufferBindTexture(DrawContext* pDrawContext, u64 handle,
                                     const ShaderLocation& rLocation, s32 textureId);
    void nvnCommandBufferBindImage(DrawContext* pDrawContext, u64 handle,
                                   const ShaderLocation& rLocation, s32 textureId);
    void invalidateGPUCacheColor(DrawContext* pDrawContext, s32 textureId) const;
    void clearCompressedFrameBufferColor(DrawContext* pDrawContext,
                                         const TextureData& rTextureData);
    void clearCompressedFrameBufferDepth(DrawContext* pDrawContext,
                                         const TextureData& rTextureData);
    void invalidateGPUCacheDepth(DrawContext* pDrawContext, s32 textureId) const;
    void nvnCommandBufferBarrier(DrawContext* pDrawContext, Barrier barrier);
    void nvnCommandBufferBarrier_Shader(DrawContext* pDrawContext, bool enable);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    void debugCallback_(const sead::GraphicsNvn::NvnDebugCallbackParam& rParam);
    s32 registerSampler_(s32 index, const NVNsampler* pSampler, u32 key);
    s32 registerTexture_(s32 index, const NVNtexture* pTexture, const NVNtextureView* pView,
                         const char* pName);
    bool offDirtyTextureCompressTexture_(u32 id);
    static s32 compareSamplerKeyU16_(const SamplerKey* pA, const u16* pKey);
    static s32 compareSamplerKey_(const SamplerKey* pA, const SamplerKey* pB);

    NVNdevice* mDevice;
    NVNqueue* mQueue;
    void* _48;
    sead::BitFlag32 mFlags;
    sead::CriticalSection mCS;
    sead::CriticalSection mCS2;
    sead::CriticalSection mSamplerCS;
    sead::CriticalSection mCS4;
    sead::Buffer<SamplerKey> mSamplers;
    u16 mSamplerIdBase;
    u16 mSamplerCursor;
    sead::PtrArray<SamplerKey> mSamplerList;
    sead::Buffer<TextureInfo> mTextures;
    u16 mTextureIdBase;
    sead::Atomic<u32> mTextureCursor;
    s16 mRegisteredSamplerNum;
    sead::Atomic<s32> mRegisteredTextureNum;
    DisplayList mDisplayList;
    sead::Atomic<u32> _400;
    sead::Atomic<s32> mCounters[5];
    sead::Delegate1<NVNMgr, const sead::GraphicsNvn::NvnDebugCallbackParam&> mDebugCallback;
    sead::FixedSafeString<64> mName;
    sead::Atomic<u64> mCopyNum;
    sead::Atomic<u64> mCopySize;
    sead::Atomic<u64> mCopyTime;
    u16 mTileWidth;
    u16 mTileHeight;
    sead::FixedSafeString<128> mFilter;
};
static_assert(sizeof(NVNMgr) == 0x548);

}  // namespace agl::driver
