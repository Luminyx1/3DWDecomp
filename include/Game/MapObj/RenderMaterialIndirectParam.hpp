#pragma once

#include <math/seadVector.h>

// Layout reconstructed from the ChameleonStateUtil interpolation routines.
struct RenderMaterialIndirectParam {
    float mBlendRate;
    float mIntensity;
    sead::Vector4f mVector0;
    sead::Vector4f mVector1;
    sead::Vector4f mVector2;
};

static_assert(sizeof(RenderMaterialIndirectParam) == 0x38);
