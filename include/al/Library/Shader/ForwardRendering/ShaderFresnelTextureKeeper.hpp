#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace agl {
class SamplerLocation;
}

namespace al {
class CurveIo;
class GpuMemAllocator;
class LutCurve;
class ShaderHolder;

/**
 * @brief Owns the fresnel, thickness and silhouette look-up curve textures.
 */
class ShaderFresnelTextureKeeper {
public:
    static s32 findFresnelId(const char* pName);
    static s32 getFresnelNum();
    static s32 getThicknessNum();

    ShaderFresnelTextureKeeper(GpuMemAllocator* pAllocator, ShaderHolder* pShaderHolder,
                               const char* pLodSettingName);
    ~ShaderFresnelTextureKeeper();

    void updateLutCurveTex();
    void activateFresnelTexture(s32 fresnelId, bool isUseDisplayList) const;
    void activateThicknessCurveTexture(s32 thicknessId, bool isUseDisplayList) const;
    void activateSilhouetteCurveTexture(s32 category, const agl::SamplerLocation& rLocation,
                                        bool isUseDisplayList) const;

private:
    sead::PtrArray<LutCurve> mFresnelCurves;
    sead::PtrArray<LutCurve> mThicknessCurves;
    sead::PtrArray<LutCurve> mSilhouetteCurves;
    CurveIo* mMapFresnelCurveIo;
    CurveIo* mObjFresnelCurveIo;
    CurveIo* mUniversalFresnelCurveIo;
    CurveIo* mWorldFresnelCurveIo;
    CurveIo* mThicknessCurveIo;
    CurveIo* mSilhouetteCurveIo;
    bool mIsLoaded = false;
};

static_assert(sizeof(ShaderFresnelTextureKeeper) == 0x68);

}  // namespace al
