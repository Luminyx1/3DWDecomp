#include "gfx/seadDrawContext.h"

#include "framework/nx/seadGameFrameworkNx.h"
#include "gfx/nin/seadGraphicsNvn.h"

namespace sead
{
/**
 * Constructs a draw context bound to the graphics gfx device.
 */
DrawContext::DrawContext()
{
    auto data = mCommandBuffer.ToData();
    data->pNnDevice = GraphicsNvn::instance()->getGfxDevice();
    data->pNvnCommandBuffer = nullptr;
    data->state = nn::gfx::CommandBufferImplData<nn::gfx::ApiVariationNvn8>::State_Begun;
}

/**
 * Destroys the draw context.
 */
DrawContext::~DrawContext()
{
    mCommandBuffer.ToData()->state =
        nn::gfx::CommandBufferImplData<nn::gfx::ApiVariationNvn8>::State_NotInitialized;
}

/**
 * Uses the framework's default NVN command buffer for this draw context.
 * @param pFramework framework (must be a GameFrameworkNx)
 */
void DrawContext::setDefaultCommandBufferFromFramework(Framework* pFramework)
{
    mCommandBuffer.ToData()->pNvnCommandBuffer =
        DynamicCast<GameFrameworkNx>(pFramework)->get158();
}

}  // namespace sead
