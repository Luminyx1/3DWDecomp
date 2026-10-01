#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Execute/IUseExecutor.hpp"

namespace agl {
class DrawContext;
}  // namespace agl

namespace agl::sdw {
class DepthShadow;
}  // namespace agl::sdw

namespace al {
class EffectSystem;

class EffectGroupDrawer : public IUseExecutor {
public:
    EffectGroupDrawer(EffectSystem* pEffectSystem, const char* pName, s32 groupId,
                      bool isEnableZSort, bool isAlwaysUpdateUbo, bool isScreenEffect);

    void execute() override;
    void drawEffectWithRenderPathAndCamPos(agl::DrawContext* pDrawContext,
                                           const sead::Matrix44f& rProjMtx,
                                           const sead::Matrix34f& rViewMtx,
                                           const sead::Vector3f& rCamPos, f32 near, f32 far,
                                           f32 fovy, u32 renderPath, bool isCalcCompute);
    void drawEffectWithRenderPath(agl::DrawContext* pDrawContext, const sead::Matrix44f& rProjMtx,
                                  const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy,
                                  u32 renderPath, bool isCalcCompute);
    void calcShadowClipVolume(agl::sdw::DepthShadow* pDepthShadow, u32 renderPath);

    const char* getName() const { return mName; }
    s32 getGroupId() const { return mGroupId; }

private:
    EffectSystem* mEffectSystem;
    const char* mName;
    s32 mGroupId;
    bool mIsEnableZSort;
    bool mIsAlwaysUpdateUbo;
    bool mIsScreenEffect;
};

static_assert(sizeof(EffectGroupDrawer) == 0x20);
}  // namespace al
