#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadDrawContext.h>
#include <nvn/nvn.h>
#include <prim/seadBitFlag.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <thread/seadCriticalSection.h>

#include "common/aglShaderEnum.h"

namespace agl {

class RenderBuffer;
class DisplayList;
enum ShaderOptimizeType {};

class DrawContext : public sead::DrawContext {
    SEAD_RTTI_OVERRIDE(DrawContext, sead::DrawContext)
public:
    DrawContext();
    ~DrawContext() override;

    void setCommandBuffer(DisplayList* pDisplayList);
    void flushCommandBuffer();
    void setBoundRenderBuffer(const RenderBuffer* pRenderBuffer);
    void barrierTexture(u32 flags);
    void barrierShader(u32 flags);
    bool isTextureDirty(u32 unused, s32 index) const;
    void setTextureDirty(s32 index);
    void changeShaderMode(ShaderMode mode, ShaderOptimizeType optimizeType);
    void setCommandBufferTemporary();

    DisplayList* getDisplayList() const { return mCommandBuffer; }
    const RenderBuffer* getBoundRenderBuffer() const { return mBoundRenderBuffer; }
    u8 getShaderMode() const { return mShaderMode; }
    u8 get_fa() const { return _fa; }
    void invalidateShaderMode() { mShaderMode = 4; }

    NVNcommandBuffer* getNvnCommandBuffer()
    {
        return static_cast<NVNcommandBuffer*>(getCommandBuffer()->ToData()->pNvnCommandBuffer);
    }

private:
    static u64 makeTextureMask_(s32 index) { return 1ull << (index & 63); }

    void setNvnCommandBuffer_(NVNcommandBuffer* pCommandBuffer)
    {
        getCommandBuffer()->ToData()->pNvnCommandBuffer = pCommandBuffer;
    }

    DisplayList* mCommandBuffer;
    const RenderBuffer* mBoundRenderBuffer;
    sead::BitFlag8 mFlags;
    u8 mShaderMode;
    u8 _fa;
    alignas(8) NVNcommandBuffer mNvnCommandBuffer;
    u64 mTextureDirty;
};
static_assert(sizeof(DrawContext) == 0x1a8);

}  // namespace agl
