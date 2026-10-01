#include "Project/Effect/EffectGroupDrawer.hpp"

#include <agl/common/aglDrawContext.h>
#include <agl/shadow/aglDepthShadow.h>
#include <math/seadGeometry.h>
#include <mc/seadCoreInfo.h>
#include <nn/util/util_VectorApi.h>

#include "Library/Effect/EffectShaderHolder.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Library/Math/MatrixUtil.hpp"

namespace al {

using Sphere = sead::Sphere<sead::Vector3f>;

/**
 * Constructs a drawer for one particle group.
 * @param pEffectSystem Effect system.
 * @param pName Name of the group.
 * @param groupId Particle group ID.
 * @param isEnableZSort Whether emitters are sorted by depth.
 * @param isAlwaysUpdateUbo Whether the uniform block is updated every frame.
 * @param isScreenEffect Whether the group is a screen effect.
 */
EffectGroupDrawer::EffectGroupDrawer(EffectSystem* pEffectSystem, const char* pName, s32 groupId,
                                     bool isEnableZSort, bool isAlwaysUpdateUbo,
                                     bool isScreenEffect)
    : mEffectSystem(pEffectSystem), mName(pName), mGroupId(groupId),
      mIsEnableZSort(isEnableZSort), mIsAlwaysUpdateUbo(isAlwaysUpdateUbo),
      mIsScreenEffect(isScreenEffect) {}

/**
 * Calculates the particles of the group.
 */
void EffectGroupDrawer::execute() {
    mEffectSystem->checkCalculateFlag(mGroupId);
    PtclSystem* ptclSystem = mEffectSystem->getPtclSystem();
    f32 frameRate = mEffectSystem->isStopCalc() ? 0.0f : 1.0f;
    ptclSystem->calc(mGroupId, frameRate);
}

/**
 * Draws the particles of the group with an explicit camera position.
 * @param pDrawContext Draw context.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param rCamPos Camera position.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param renderPath Render path flags.
 * @param isCalcCompute Whether the view parameters are set up before drawing.
 */
void EffectGroupDrawer::drawEffectWithRenderPathAndCamPos(
    agl::DrawContext* pDrawContext, const sead::Matrix44f& rProjMtx,
    const sead::Matrix34f& rViewMtx, const sead::Vector3f& rCamPos, f32 near, f32 far, f32 fovy,
    u32 renderPath, bool isCalcCompute) {
    if (isCalcCompute) {
        mEffectSystem->getPtclSystem()->beginRender(pDrawContext->getCommandBuffer(), rProjMtx,
                                                    rViewMtx, rCamPos, near, far, fovy);
    }

    EffectSystem* effectSystem = mEffectSystem;
    s32 groupId = mGroupId;
    bool isEnableZSort = mIsEnableZSort;
    bool isDoComputeShaderProcess = !EffectSystem::isEnableBatchCompute();
    EffectShaderHolder* shaderHolder = effectSystem->getShaderHolder();
    PtclSystem* ptclSystem = effectSystem->getPtclSystem();
    shaderHolder->bindCustomShaderUbo(pDrawContext);
    ptclSystem->Draw(sead::CoreInfo::getCurrentCoreId(), pDrawContext->getCommandBuffer(),
                     groupId, renderPath, isEnableZSort, isDoComputeShaderProcess,
                     effectSystem->getShaderHolder());
}

/**
 * Draws the particles of the group.
 * @param pDrawContext Draw context.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param renderPath Render path flags.
 * @param isCalcCompute Whether the view parameters are set up before drawing.
 */
void EffectGroupDrawer::drawEffectWithRenderPath(agl::DrawContext* pDrawContext,
                                                 const sead::Matrix44f& rProjMtx,
                                                 const sead::Matrix34f& rViewMtx, f32 near,
                                                 f32 far, f32 fovy, u32 renderPath,
                                                 bool isCalcCompute) {
    sead::Vector3f camPos;
    calcCameraPosFromViewMtx(&camPos, rViewMtx);
    drawEffectWithRenderPathAndCamPos(pDrawContext, rProjMtx, rViewMtx, camPos, near, far, fovy,
                                      renderPath, isCalcCompute);
}

/**
 * Updates the clip volume of a depth shadow with the bounding spheres of the group's emitter sets.
 * @param pDepthShadow Depth shadow.
 * @param renderPath Render path flags.
 */
void EffectGroupDrawer::calcShadowClipVolume(agl::sdw::DepthShadow* pDepthShadow,
                                             u32 renderPath) {
    for (nn::vfx::EmitterSet* emitterSet =
             mEffectSystem->getPtclSystem()->GetEmitterSetHead(mGroupId);
         emitterSet != nullptr; emitterSet = emitterSet->GetNext()) {
        if (!emitterSet->IsDrawEnable() || (emitterSet->GetDrawPathFlag() & renderPath) == 0) {
            continue;
        }

        u32 clipRadius = emitterSet->GetEmitterSetResource()->m_ResEmitterSet->m_ClipRadius;
        if (clipRadius == 0) {
            continue;
        }

        f32 radius = clipRadius;
        const nn::util::Vector3fType& pos = emitterSet->GetClipPos();
        Sphere sphere(sead::Vector3f(nn::util::VectorGetX(pos), nn::util::VectorGetY(pos),
                                     nn::util::VectorGetZ(pos)),
                      radius);
        pDepthShadow->checkAndUpdate(sphere, sead::CoreInfo::getCurrentCoreId(), 0);
    }
}

}  // namespace al
