#pragma once

#include <math/seadVector.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureEnum.h"
#include "common/aglTextureSampler.h"

namespace sead {
class Heap;
}

namespace agl {

class DrawContext;
class TextureData;

class TextureDataImageAccessor {
public:
    TextureDataImageAccessor();
    virtual ~TextureDataImageAccessor();

    void initializeImageBuffer(const TextureData& rTexture, sead::Heap* pHeap);
    void finalizeImageBuffer();
    void setLinearTexture(TextureData* pTexture);
    void updateImageBuffer(DrawContext* pDrawContext, const TextureData& rTexture) const;
    void beginPeek(DrawContext* pDrawContext, const TextureData& rTexture, sead::Heap* pHeap);
    void sync(DrawContext* pDrawContext) const;
    void peek(sead::Vector4f* pColor, s32 x, s32 y) const;
    void peek(sead::Vector4<u32>* pColor, s32 x, s32 y) const;

private:
    void updateLinearInfo_(const TextureData& rTexture);
    void syncMemoryGL_(const TextureData& rTexture) const;

    GPUMemVoidAddr mImageAddr;
    TextureFormat mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    u32 mStride = 0;
    mutable RenderBuffer mRenderBuffer;
    mutable TextureSampler mSampler;
    mutable RenderTargetColor mRenderTarget;
    bool mIsLinearTextureSet = false;
};
static_assert(sizeof(TextureDataImageAccessor) == 0x380);

}  // namespace agl
