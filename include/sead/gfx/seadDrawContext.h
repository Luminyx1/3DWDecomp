#pragma once

#include <nn/gfx/gfx_CommandBuffer.h>
#include <nn/gfx/gfx_Types.h>
#include <nvn/nvn.h>
#include "basis/seadTypes.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead
{
class Framework;

class DrawContext
{
    SEAD_RTTI_BASE(DrawContext)
public:
    DrawContext();
    virtual ~DrawContext();

    void setDefaultCommandBufferFromFramework(Framework* pFramework);

    nn::gfx::CommandBuffer* getCommandBuffer() { return &mCommandBuffer; }

    NVNcommandBuffer* getNvnCommandBuffer() const
    {
        return static_cast<NVNcommandBuffer*>(mCommandBuffer.ToData()->pNvnCommandBuffer.ptr);
    }

private:
    nn::gfx::CommandBuffer mCommandBuffer;
};
static_assert(sizeof(DrawContext) == 0xE8);

}  // namespace sead
