#pragma once
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResAnimCurve.h>

namespace nn::g3d {
struct ResShaderParamAnimInfo {
    nn::util::BinPtrToString name;
    u16 firstCurve;
    u16 floatCount;
    u16 intCount;
    u8 _e[4];
    u16 bindIndex;
    u8 _14[4];
};
struct ResTexturePatternAnimInfo {
    nn::util::BinPtrToString name;
    u16 curveIndex;
    u16 _a;
    u8 bindIndex;
    u8 _d[3];
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
    u8 _20[0xe];
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
