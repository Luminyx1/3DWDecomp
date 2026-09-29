#include "common/aglDrawContext.h"

#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDisplayList.h"
#include "driver/aglNVNMgr.h"

namespace agl {

/**
 * Constructs a draw context that records into the NVN manager's default command buffer.
 */
DrawContext::DrawContext()
    : mCommandBuffer(nullptr), mBoundRenderBuffer(nullptr), _fa(0)
{
    mFlags.makeAllZero();
    mShaderMode = 4;
    mTextureDirty = 0;
    setNvnCommandBuffer_(
        static_cast<NVNcommandBuffer*>(driver::NVNMgr::instance()->_48));
}

/**
 * Sets the display list to record into, or reverts to the default command buffer.
 * @param pDisplayList display list to record into, or nullptr
 */
void DrawContext::setCommandBuffer(DisplayList* pDisplayList)
{
    mCommandBuffer = pDisplayList;
    if (pDisplayList)
    {
        setNvnCommandBuffer_(&mNvnCommandBuffer);
        pDisplayList->mNvnCommandBuffer = &mNvnCommandBuffer;
    }
    else
    {
        setNvnCommandBuffer_(static_cast<NVNcommandBuffer*>(driver::NVNMgr::instance()->_48));
    }
}

/**
 * Flushes the temporary command buffer if one is active.
 */
DrawContext::~DrawContext()
{
    if (mFlags.isOnBit(0))
    {
        flushCommandBuffer();
    }
}

/**
 * Ends recording into the current display list and submits it.
 */
void DrawContext::flushCommandBuffer()
{
    mCommandBuffer->endDisplayList();
    if (mCommandBuffer->isValid())
    {
        mCommandBuffer->callDirect(this);
    }

    if (mFlags.isOnBit(0))
    {
        driver::NVNMgr::instance()->mCS4.unlock();
    }
    mFlags.resetBit(0);
}

/**
 * Sets the render buffer that is currently bound.
 * @param pRenderBuffer bound render buffer
 */
void DrawContext::setBoundRenderBuffer(const RenderBuffer* pRenderBuffer)
{
    mBoundRenderBuffer = pRenderBuffer;
}

/**
 * Inserts a texture barrier and clears all texture dirty flags.
 * @param flags barrier flags (1: order primitives, 2: order fragments, 4: order indirect data)
 */
void DrawContext::barrierTexture(u32 flags)
{
    if (flags == 0)
    {
        return;
    }

    int barrier = NVN_BARRIER_INVALIDATE_TEXTURE_BIT;
    if (flags & 1)
    {
        barrier |= NVN_BARRIER_ORDER_PRIMITIVES_BIT;
    }
    if (flags & 2)
    {
        barrier |= NVN_BARRIER_ORDER_FRAGMENTS_BIT;
    }
    if (flags & 4)
    {
        barrier |= NVN_BARRIER_ORDER_INDIRECT_DATA_BIT;
    }
    nvnCommandBufferBarrier(getNvnCommandBuffer(), barrier);
    mTextureDirty = 0;
}

/**
 * Inserts a shader barrier.
 * @param flags barrier flags (8: also order indirect data)
 */
void DrawContext::barrierShader(u32 flags)
{
    if (flags == 0)
    {
        return;
    }

    driver::NVNMgr::instance()->nvnCommandBufferBarrier_Shader(this, (flags >> 3) & 1);
}

/**
 * Checks whether a texture slot is marked dirty.
 * @param unused unused
 * @param index texture slot index
 * @return whether the slot is dirty
 */
bool DrawContext::isTextureDirty(u32 unused, s32 index) const
{
    return (makeTextureMask_(index) & mTextureDirty) != 0;
}

/**
 * Marks a texture slot dirty.
 * @param index texture slot index
 */
void DrawContext::setTextureDirty(s32 index)
{
    mTextureDirty |= makeTextureMask_(index);
}

/**
 * Sets the shader mode.
 * @param mode shader mode
 * @param optimizeType unused
 */
void DrawContext::changeShaderMode(ShaderMode mode, ShaderOptimizeType optimizeType)
{
    mShaderMode = mode;
}

/**
 * Locks the NVN manager and records into its shared display list until the next flush.
 */
void DrawContext::setCommandBufferTemporary()
{
    driver::NVNMgr::instance()->mCS4.lock();
    DisplayList* pDisplayList = &driver::NVNMgr::instance()->mDisplayList;
    setCommandBuffer(pDisplayList);
    pDisplayList->beginDisplayList();
    mFlags.setBit(0);
}

}  // namespace agl
