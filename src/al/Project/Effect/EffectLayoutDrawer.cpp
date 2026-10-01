#include "Project/Effect/EffectLayoutDrawer.hpp"

#include <agl/common/aglDrawContext.h>
#include <gfx/seadProjection.h>
#include <mc/seadCoreInfo.h>

#include "Library/Effect/EffectShaderHolder.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"

namespace al {

static inline void drawGroup(PtclSystem* pPtclSystem, nn::gfx::CommandBuffer* pCommandBuffer,
                             s32 groupId, u32 renderPath, bool isDoComputeShaderProcess,
                             const EffectSystem* pEffectSystem) {
    pPtclSystem->Draw(sead::CoreInfo::getCurrentCoreId(), pCommandBuffer, groupId, renderPath,
                      false, isDoComputeShaderProcess, pEffectSystem->getShaderHolder());
}

/**
 * Constructs a drawer for a layout particle group.
 * @param pEffectSystem Effect system.
 * @param pName Name of the group.
 * @param groupId Particle group ID.
 * @param renderPath Render path flags.
 */
EffectLayoutDrawer::EffectLayoutDrawer(EffectSystem* pEffectSystem, const char* pName,
                                       s32 groupId, u32 renderPath)
    : mEffectSystem(pEffectSystem), mName(pName), mGroupId(groupId), mRenderPath(renderPath) {}

/**
 * Draws the particles of the group with an orthographic projection covering the layout screen.
 */
void EffectLayoutDrawer::draw() const {
    mEffectSystem->getDrawContext()->changeShaderMode(static_cast<agl::ShaderMode>(1),
                                                      static_cast<agl::ShaderOptimizeType>(0));
    f32 width = getLayoutDisplayWidth();
    f32 height = getLayoutDisplayHeight();
    sead::OrthoProjection projection(-5000.0f, 5000.0f, height * 0.5f, height * -0.5f,
                                     width * -0.5f, width * 0.5f);
    sead::Matrix44f projMtx = projection.getProjectionMatrix();
    sead::Matrix34f viewMtx(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                            0.0f);
    drawEffect(mEffectSystem->getDrawContext(), projMtx, viewMtx, -5000.0f, 5000.0f, 1.57f);
}

/**
 * Draws the particles of the group.
 * @param pDrawContext Draw context.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 */
void EffectLayoutDrawer::drawEffect(agl::DrawContext* pDrawContext,
                                    const sead::Matrix44f& rProjMtx,
                                    const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                                    f32 fovy) const {
    sead::Vector3f camPos;
    PtclSystem* ptclSystem = mEffectSystem->getPtclSystem();
    calcCameraPosFromViewMtx(&camPos, rViewMtx);
    ptclSystem->beginRender(pDrawContext->getCommandBuffer(), rProjMtx, rViewMtx, camPos, near,
                            far, fovy);

    EffectSystem* effectSystem = mEffectSystem;
    s32 groupId = mGroupId;
    u32 renderPath = mRenderPath;
    bool isDoComputeShaderProcess = !EffectSystem::isEnableBatchCompute();
    EffectShaderHolder* shaderHolder = effectSystem->getShaderHolder();
    PtclSystem* drawPtclSystem = effectSystem->getPtclSystem();
    shaderHolder->bindCustomShaderUbo(pDrawContext);
    drawGroup(drawPtclSystem, pDrawContext->getCommandBuffer(), groupId, renderPath,
              isDoComputeShaderProcess, effectSystem);
}

/**
 * Does nothing.
 */
void EffectLayoutDrawer::execute() {}

}  // namespace al
