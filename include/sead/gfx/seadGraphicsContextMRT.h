#pragma once

#include <gfx/seadGraphicsContext.h>

namespace sead
{
class GraphicsContextMRT : public GraphicsContext
{
public:
    GraphicsContextMRT() = default;
};
static_assert(sizeof(GraphicsContextMRT) == 0x74);

}  // namespace sead
