#pragma once

#include <nn/g3d/g3d_Bounding.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/util/util_MathTypes.h>

namespace nn::g3d {

struct CullingContext {
    int nodeIndex = 0;
    int nodeCount = -1;
    int submeshIndex = 0;
    int submeshCount = 0;
    int _10 = 0;
};

static_assert(sizeof(ShapeObj) == 0x70);

}  // namespace nn::g3d
