#pragma once
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResAnimCurve.h>

namespace nn::g3d {
struct ResShaderParamAnimInfo {
    nn::util::BinPtrToString name;
    u16 firstCurve;
    u16 floatCount;
    u16 intCount;
    u16 firstConstant;
    u16 constantCount;
    u16 bindIndex;
    u8 _14[4];
};
struct ResTexturePatternAnimInfo {
    nn::util::BinPtrToString name;
    u16 curveIndex;
    u16 baseValueIndex;
    u8 bindIndex;
    u8 _d[3];
};
struct ResAnimConstant {
    u32 targetOffset;
    u32 value;
};
class ResPerMaterialAnim {
public:
    template <bool cached> void EvaluateShaderParamAnim(void* result, float frame, const u16* indices, AnimFrameCache* cache) const;
    template <bool cached> void EvaluateTexturePatternAnim(int* result, float frame, const u16* indices, AnimFrameCache* cache) const;
    template <bool cached> void EvaluateVisibilityAnim(int* result, float frame, AnimFrameCache* cache) const;
    BindResult PreBind(const ResMaterial* material);
    BindResult BindCheck(const ResMaterial* material) const;

    nn::util::BinPtrToString name;
    ResShaderParamAnimInfo* parameters;
    ResTexturePatternAnimInfo* textures;
    ResAnimCurve* curves;
    const ResAnimConstant* constants;
    u16 shaderParamResultIndex;
    u16 texturePatternResultIndex;
    u16 visibilityResultIndex;
    u16 visibilityCurve;
    u16 visibilityBase;
    u16 parameterCount;
    u16 textureCount;
    u16 _36;
    u16 curveCount;
    u8 _3a[6];
};
static_assert(sizeof(ResPerMaterialAnim) == 0x40, "Per-material animation resource size");
}
