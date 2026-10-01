#include "Project/Effect/Core/EffectKeeper.hpp"

#include <cstring>

#include <agl/common/aglDrawContext.h>
#include <gfx/seadGraphics.h>
#include <gfx/seadViewport.h>

#include <nn/util/util_MathTypes.h>

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Effect/EffectShaderHolder.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/EffectSystemInfo.hpp"
#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Library/Layout/LayoutActorUtil.hpp"
#include "Library/Math/MatrixPtrHolder.hpp"
#include "Library/Model/JointMtxPtr.hpp"
#include "Library/Model/ModelShapeUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Core/IUseCameraDirector.hpp"
#include "Project/Effect/Core/IUseEffectKeeper.hpp"
#include "Project/Effect/Effect.hpp"
#include "Project/Effect/EffectCameraHolder.hpp"
#include "Project/Effect/EffectEmitter.hpp"
#include "Project/Effect/EffectInfo.hpp"

namespace nn::util::MatrixRowMajor4x3f {
extern const neon::MatrixRowMajor4x3fType ConstantIdentity;
}  // namespace nn::util::MatrixRowMajor4x3f

namespace {

const char* const cGroup3D = "エフェクト（３Ｄ）";
const char* const cGroupPlayer = "エフェクト（プレイヤー）";
const char* const cGroupHitStop = "Effect (HitStop)";
const char* const cGroupCameraDemo = "エフェクト（カメラデモ）";
const char* const cGroupZSort = "エフェクト（Ｚソート）";
const char* const cGroup2D = "エフェクト（２Ｄ）";

const sead::Matrix34f* findJointMtxPtr(const al::EffectKeeper* pEffectKeeper,
                                       const al::ModelKeeper* pModelKeeper, const char* pName) {
    if (pEffectKeeper->getMtxPtrHolder() != nullptr) {
        const sead::Matrix34f* mtx = pEffectKeeper->getMtxPtrHolder()->tryFindMtxPtr(pName);
        if (mtx != nullptr) {
            return mtx;
        }
    }

    return al::getJointMtxPtrByIndex(pModelKeeper, al::getJointIndex(pModelKeeper, pName));
}

bool isExistFarClipEffect(const al::EffectKeeper* pEffectKeeper) {
    for (s32 i = 0; i < pEffectKeeper->getEffectNum(); i++) {
        if (pEffectKeeper->getEffect(i)->getEffectInfo()->mParam.mFarClipDistance > 0.0f) {
            return true;
        }
    }

    return false;
}

al::Effect* tryFindEffectByName(const al::EffectKeeper* pEffectKeeper, const char* pName) {
    s32 effectNum = pEffectKeeper->getEffectNum();

    for (s32 i = 0; i < effectNum; i++) {
        al::Effect* effect = pEffectKeeper->getEffect(i);
        if (al::isEqualString(pName, effect->getName())) {
            return effect;
        }
    }

    return nullptr;
}

void makeScreenProjection(nn::util::MatrixT4x4fType* pProjMtx, f32 width, f32 height,
                          f32 near, f32 far) {
    f32 left = -width * 0.5f;
    f32 right = width * 0.5f;
    f32 bottom = -height * 0.5f;
    f32 top = height * 0.5f;
    f32 invWidth = 1.0f / (right - left);
    f32 invHeight = 1.0f / (top - bottom);
    f32 invDepth = 1.0f / (near - far);

    pProjMtx->_m.val[0] = float32x4_t{2.0f * invWidth, 0.0f, 0.0f, 0.0f};
    pProjMtx->_m.val[1] = float32x4_t{0.0f, 2.0f * invHeight, 0.0f, 0.0f};
    pProjMtx->_m.val[2] = float32x4_t{0.0f, 0.0f, invDepth, 0.0f};
    pProjMtx->_m.val[3] = float32x4_t{-(left + right) * invWidth, -(bottom + top) * invHeight,
                                      near * invDepth, 1.0f};
}

}  // namespace

namespace alEffectKeeperInitFunction {

/**
 * Binds the effects and emitters of an effect keeper to the joints of a model.
 * @param pEffectKeeper Effect keeper.
 * @param pModelKeeper Model keeper providing the joint matrices.
 */
void setupModelToEffectKeeper(al::EffectKeeper* pEffectKeeper,
                              const al::ModelKeeper* pModelKeeper) {
    for (s32 i = 0; i < pEffectKeeper->getEffectNum(); i++) {
        al::Effect* effect = pEffectKeeper->getEffect(i);
        const char* jointName = effect->getEffectInfo()->mParam.mJointName;

        if (jointName != nullptr) {
            const sead::Matrix34f* mtx = findJointMtxPtr(pEffectKeeper, pModelKeeper, jointName);
            if (mtx != nullptr) {
                effect->initMtxPtr(al::JointMtxPtr(mtx));
            }
        }

        s32 emitterNum = effect->getEmitterNum();

        for (s32 j = 0; j < emitterNum; j++) {
            al::EffectEmitter* emitter = effect->getEmitter(j);
            const char* emitterJointName = emitter->getResourceInfo()->mJointName;

            if (emitterJointName == nullptr) {
                continue;
            }

            const sead::Matrix34f* mtx =
                findJointMtxPtr(pEffectKeeper, pModelKeeper, emitterJointName);
            if (mtx != nullptr) {
                emitter->initMtxPtr(al::JointMtxPtr(mtx));
            }
        }
    }
}

/**
 * Binds the effects of an effect keeper to the panes of a layout.
 * @param pEffectKeeper Effect keeper.
 * @param pLayout Layout providing the pane matrices.
 */
void setupLayoutToEffectKeeper(al::EffectKeeper* pEffectKeeper, const al::IUseLayout* pLayout) {
    for (s32 i = 0; i < pEffectKeeper->getEffectNum(); i++) {
        al::Effect* effect = pEffectKeeper->getEffect(i);
        const char* jointName = effect->getEffectInfo()->mParam.mJointName;

        if (jointName != nullptr) {
            effect->initMtxPtr(al::JointMtxPtr(al::getPaneMtxRaw(pLayout, jointName)));
        }
    }
}

/**
 * Gives every effect of an effect keeper a camera holder for the given camera.
 * @param pEffectKeeper Effect keeper.
 * @param pCamera Camera user.
 */
void setupCameraToEffectKeeper(al::EffectKeeper* pEffectKeeper, const al::IUseCamera* pCamera) {
    al::getCameraViewMtxPtr(pCamera);
    auto* cameraHolder = new al::EffectCameraHolder();
    cameraHolder->setSceneCameraInfo(pCamera->getSceneCameraInfo());

    for (s32 i = 0; i < pEffectKeeper->getEffectNum(); i++) {
        pEffectKeeper->getEffect(i)->setCameraHolder(cameraHolder);
    }
}

/**
 * Rebinds the effects and emitters attached to a named matrix.
 * @param pEffectKeeper Effect keeper.
 * @param pName Name of the matrix.
 */
void updateNamedMtxPtr(al::EffectKeeper* pEffectKeeper, const char* pName) {
    const sead::Matrix34f* mtx = pEffectKeeper->getMtxPtrHolder()->findMtxPtr(pName);

    for (s32 i = 0; i < pEffectKeeper->getEffectNum(); i++) {
        al::Effect* effect = pEffectKeeper->getEffect(i);
        bool isUpdated = false;
        const char* jointName = effect->getEffectInfo()->mParam.mJointName;

        if (jointName != nullptr && al::isEqualString(jointName, pName)) {
            effect->initMtxPtr(al::JointMtxPtr(mtx));
            isUpdated = true;
        }

        s32 emitterNum = effect->getEmitterNum();

        for (s32 j = 0; j < emitterNum; j++) {
            al::EffectEmitter* emitter = effect->getEmitter(j);
            const char* emitterJointName = emitter->getResourceInfo()->mJointName;

            if (emitterJointName != nullptr && al::isEqualString(emitterJointName, pName)) {
                emitter->updateMtxPtr(al::JointMtxPtr(mtx));
                isUpdated = true;
            }
        }

        if (isUpdated) {
            effect->update();
        }
    }
}

}  // namespace alEffectKeeperInitFunction

namespace alEffectSystemFunction {

/**
 * Selects the single render target draw path.
 * @param pEffectSystem Effect system.
 * @param isEnable Whether depth shadows are drawn.
 */
void setDrawPathRenderStateSetCallbackSRT(const al::EffectSystem* pEffectSystem, bool isEnable) {
    pEffectSystem->getShaderHolder()->setDrawPathRenderStateSetCallbackSRT(isEnable);
}

/**
 * Runs the compute pass of the effect system.
 * @param pEffectSystem Effect system.
 */
void calcEffectCompute(const al::EffectSystem* pEffectSystem) {
    pEffectSystem->calcEffectCompute();
}

/**
 * Selects the multiple render target draw path.
 * @param pEffectSystem Effect system.
 */
void setDrawPathRenderStateSetCallbackMRT(const al::EffectSystem* pEffectSystem) {
    pEffectSystem->getShaderHolder()->setDrawPathRenderStateSetCallbackMRT();
}

/**
 * Draws the deferred shadow mask effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectDeferredShadowMask(const al::EffectSystem* pEffectSystem,
                                  const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                                  f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x20,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x20, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupHitStop,
                                            0x20, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x20, false);
}

/**
 * Draws the deferred shadow mask effects with an explicit camera position.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param rCamPos Camera position.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectDeferredShadowMaskWithPos(const al::EffectSystem* pEffectSystem,
                                         const sead::Matrix44f& rProjMtx,
                                         const sead::Matrix34f& rViewMtx,
                                         const sead::Vector3f& rCamPos, f32 near, f32 far,
                                         f32 fovy) {
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroup3D, 0x20, true);
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroupPlayer, 0x20, false);
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroupHitStop, 0x20, false);
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroupCameraDemo, 0x20, false);
}

/**
 * Draws the deferred shadow mask effects of the player group.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectDeferredShadowMaskPlayer(const al::EffectSystem* pEffectSystem,
                                        const sead::Matrix44f& rProjMtx,
                                        const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                                        f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x20, true);
}

/**
 * Draws the deferred effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectDeferred(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 1,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer, 1,
                                            false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupHitStop, 1,
                                            false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 1, false);
}

/**
 * Draws the deferred effects with an explicit camera position.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param rCamPos Camera position.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectDeferredWithPos(const al::EffectSystem* pEffectSystem,
                               const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                               const sead::Vector3f& rCamPos, f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroup3D, 1, true);
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroupPlayer, 1, false);
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroupHitStop, 1, false);
    pEffectSystem->drawEffectWithRenderPathAndCamPos(rProjMtx, rViewMtx, rCamPos, near, far, fovy,
                                                     cGroupCameraDemo, 1, false);
}

/**
 * Draws the deferred effects of the player group.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectDeferredPlayer(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer, 1,
                                            true);
}

/**
 * Draws the forward effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectForward(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                       const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x10,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 2,
                                            false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer, 2,
                                            false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupHitStop, 2,
                                            false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 2, false);
}

/**
 * Draws the forward effects of the player group.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectForwardPlayer(const al::EffectSystem* pEffectSystem,
                             const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                             f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer, 2,
                                            true);
}

/**
 * Draws the forward effects that are rendered after fog.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectForwardAfterFog(const al::EffectSystem* pEffectSystem,
                               const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                               f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x400,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x400, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupHitStop,
                                            0x400, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x400, false);
}

/**
 * Draws the forward effects rendered into the reduced buffer.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectForwardReduced(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x100,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x100, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x100, false);
}

/**
 * Draws the forward effects rendered into the HDR reduced buffer.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectForwardReducedHDR(const al::EffectSystem* pEffectSystem,
                                 const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                                 f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x200,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x200, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x200, false);
}

/**
 * Draws the indirect effects, then the 2D indirect effects with a screen projection.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 * @param pViewport Viewport giving the 2D screen size.
 */
void drawEffectIndirect(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy,
                        const sead::Viewport* pViewport) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 4,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer, 4,
                                            false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 4, false);

    nn::util::MatrixT4x4fType projMtx;
    makeScreenProjection(&projMtx, pViewport->getSizeX(), pViewport->getSizeY(), -5000.0f,
                         5000.0f);
    nn::util::Matrix4x3fType viewMtx = nn::util::MatrixRowMajor4x3f::ConstantIdentity;

    pEffectSystem->drawEffectWithRenderPath(*reinterpret_cast<const sead::Matrix44f*>(&projMtx),
                                            *reinterpret_cast<const sead::Matrix34f*>(&viewMtx),
                                            -5000.0f, 5000.0f, fovy, cGroup2D, 4, true);
}

/**
 * Draws the indirect effects of the player group.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 * @param pViewport Viewport (unused).
 */
void drawEffectIndirectPlayer(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy, const sead::Viewport* pViewport) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer, 4,
                                            true);
}

/**
 * Draws the post effect background effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectPostEffectBackground(const al::EffectSystem* pEffectSystem,
                                    const sead::Matrix44f& rProjMtx,
                                    const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x4000,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x4000, false);
}

/**
 * Draws the post effect effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectPostEffect(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                          const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D,
                                            0x18000, true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x18000, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x18000, false);
}

/**
 * Draws the effects rendered after fog.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectAfterFog(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x400,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x400, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x400, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupHitStop,
                                            0x400, false);
}

/**
 * Draws the shadow caster effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectShadowCaster(const al::EffectSystem* pEffectSystem,
                            const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                            f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x80,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x80, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x80, false);
}

/**
 * Draws the shadow receiver effects.
 * @param pEffectSystem Effect system.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip.
 * @param far Far clip.
 * @param fovy Vertical field of view.
 */
void drawEffectShadowReceiver(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy) {
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroup3D, 0x40,
                                            true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy, cGroupPlayer,
                                            0x40, false);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, near, far, fovy,
                                            cGroupCameraDemo, 0x40, false);
}

/**
 * Draws the 2D effects with a projection covering the layout display.
 * @param pEffectSystem Effect system.
 * @param pViewport Viewport (unused).
 */
void drawEffect2D(const al::EffectSystem* pEffectSystem, const sead::Viewport* pViewport) {
    pEffectSystem->getDrawContext()->changeShaderMode(agl::cShaderMode_UniformBlock,
                                                      agl::ShaderOptimizeType(0));

    nn::util::MatrixT4x4fType projMtx;
    makeScreenProjection(&projMtx, al::getLayoutDisplayWidth(), al::getLayoutDisplayHeight(),
                         -5000.0f, 5000.0f);
    nn::util::Matrix4x3fType viewMtx = nn::util::MatrixRowMajor4x3f::ConstantIdentity;
    const auto& rProjMtx = *reinterpret_cast<const sead::Matrix44f*>(&projMtx);
    const auto& rViewMtx = *reinterpret_cast<const sead::Matrix34f*>(&viewMtx);

    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, -5000.0f, 5000.0f, 1.57f, cGroup2D,
                                            1, true);
    pEffectSystem->drawEffectWithRenderPath(rProjMtx, rViewMtx, -5000.0f, 5000.0f, 1.57f, cGroup2D,
                                            2, false);
}

/**
 * Updates the 2D effect group.
 * @param pEffectSystem Effect system.
 */
void updateEffect2D(al::EffectSystem* pEffectSystem) {
    pEffectSystem->preprocess();
    pEffectSystem->updateEffect(cGroup2D);
    pEffectSystem->postprocess();
}

/**
 * Calculates the shadow clip volume of the shadow casting effects.
 * @param pEffectSystem Effect system.
 * @param pDepthShadow Depth shadow to update.
 */
void calcShadowClipVolume(const al::EffectSystem* pEffectSystem,
                          agl::sdw::DepthShadow* pDepthShadow) {
    pEffectSystem->calcShadowClipVolume(pDepthShadow, cGroup3D, 0x80);
    pEffectSystem->calcShadowClipVolume(pDepthShadow, cGroupPlayer, 0x80);
    pEffectSystem->calcShadowClipVolume(pDepthShadow, cGroupZSort, 0x80);
    pEffectSystem->calcShadowClipVolume(pDepthShadow, cGroupCameraDemo, 0x80);
}

/**
 * Does nothing.
 * @param pSystemInfo Effect system info (unused).
 */
void tryDeleteEmitterAndParticleOneTime(const al::EffectSystemInfo* pSystemInfo) {}

/**
 * Sets the depth texture used by the effect shaders.
 * @param pEffectSystem Effect system.
 * @param pTexture Depth texture.
 */
void setDepthTexture(const al::EffectSystem* pEffectSystem, const agl::TextureData* pTexture) {
    pEffectSystem->getShaderHolder()->setupTextureDepth(pTexture);
}

/**
 * Checks whether an emitter is rendered into the reduced buffer.
 * @param pEffectSystem Effect system.
 * @return Whether such an emitter exists.
 */
bool isHasRenderingEmitterInReduceBuffer(const al::EffectSystem* pEffectSystem) {
    return pEffectSystem->isHasRenderingEmitter(0x100);
}

/**
 * Checks whether an emitter is rendered into the HDR reduced buffer.
 * @param pEffectSystem Effect system.
 * @return Whether such an emitter exists.
 */
bool isHasRenderingEmitterInReduceBufferHdr(const al::EffectSystem* pEffectSystem) {
    return pEffectSystem->isHasRenderingEmitter(0x200);
}

}  // namespace alEffectSystemFunction

namespace alEffectFunction {

/**
 * Binary searches the effect data base for a user.
 * @param pSystemInfo Effect system info.
 * @param pName Name of the user.
 * @return The user, or nullptr if there is none.
 */
__attribute__((noinline)) al::EffectUserInfo* tryFindEffectUser(const al::EffectSystemInfo* pSystemInfo,
                                      const char* pName) {
    const al::EffectDataBase* dataBase = pSystemInfo->mEffectDataBase;
    s32 low = 0;
    s32 high = dataBase->mUserNum - 1;
    al::EffectUserInfo** users = dataBase->mUsers;

    while (low < high) {
        s32 mid = (low + high) / 2;
        al::EffectUserInfo* user = users[mid];
        s32 result = std::strcmp(user->mName, pName);

        if (result == 0) {
            return user;
        }

        if (result < 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    al::EffectUserInfo* user = users[low];
    return std::strcmp(user->mName, pName) == 0 ? user : nullptr;
}

/**
 * Looks up the emitter set resource of an effect resource info.
 * @param pSystemInfo Effect system info.
 * @param pResourceInfo Resource info to initialize.
 */
void initResourceInfo(const al::EffectSystemInfo* pSystemInfo,
                      al::EffectResourceInfo* pResourceInfo) {
    pResourceInfo->mEmitterSetResourceInfo = tryFindEffectResouceInfo(pSystemInfo,
                                                                      pResourceInfo->mName);
}

/**
 * Looks up an emitter set resource by name.
 * @param pSystemInfo Effect system info.
 * @param pName Name of the emitter set.
 * @return The resource info, or nullptr if there is none.
 */
al::EmitterSetResourceInfo* tryFindEffectResouceInfo(const al::EffectSystemInfo* pSystemInfo,
                                                     const char* pName) {
    return pSystemInfo->mPtclSystem->getEmitterSetResourceInfoHolder()->tryFindEffectResouceInfo(
        pName);
}

/**
 * Emits an effect if the effect keeper has it.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param pPos Emit position.
 * @return Whether the effect exists.
 */
bool emitEffectIfExist(al::IUseEffectKeeper* pUser, const char* pName,
                       const sead::Vector3f* pPos) {
    al::Effect* effect = tryFindEffectByName(pUser->getEffectKeeper(), pName);

    if (effect == nullptr) {
        return false;
    }

    effect->emitEmitters(pPos, false);
    return true;
}

}  // namespace alEffectFunction

namespace al {

/**
 * Creates the effects of the effect user with the given name.
 * @param pSystemInfo Effect system info.
 * @param pName Name of the effect user.
 * @param pTrans Translation the effects follow.
 * @param pScale Scale the effects follow.
 * @param pMtx Matrix the effects follow.
 */
EffectKeeper::EffectKeeper(const EffectSystemInfo* pSystemInfo, const char* pName,
                           const sead::Vector3f* pTrans, const sead::Vector3f* pScale,
                           const sead::Matrix34f* pMtx)
    : mName(pName) {
    mEffectUserInfo = alEffectFunction::tryFindEffectUser(pSystemInfo, pName);
    if (mEffectUserInfo != nullptr) {
        mEffectNum = mEffectUserInfo->mEffectNum;
    }

    mEffects = new Effect*[mEffectNum];

    for (s32 i = 0; i < mEffectNum; i++) {
        const EffectInfo* info = &mEffectUserInfo->mEffects[i];
        mEffects[i] =
            new Effect(pSystemInfo, info, pTrans, pScale, pMtx, reinterpret_cast<u64>(this));
    }

    mIsExistFarClipEffect = isExistFarClipEffect(this);

    if (mEffectUserInfo == nullptr || mEffectUserInfo->mNamedMtxList == nullptr) {
        return;
    }

    mMtxPtrHolder = new MtxPtrHolder();
    s32 namedMtxNum = mEffectUserInfo->mNamedMtxList->mNum;
    mMtxPtrHolder->init(namedMtxNum);

    for (s32 i = 0; i < namedMtxNum; i++) {
        mMtxPtrHolder->setMtxPtrAndName(i, mEffectUserInfo->mNamedMtxList->mNames[i], pMtx);
    }
}

/**
 * Updates the effects while any of them is emitted or far clipped.
 */
void EffectKeeper::update() {
    if (!mIsEmitted && !mIsExistFarClipEffect) {
        return;
    }

    bool isActive = false;

    for (s32 i = 0; i < mEffectNum; i++) {
        isActive |= mEffects[i]->update();
    }

    mIsEmitted = isActive;
}

/**
 * Switches the effects to a new material code.
 * @param pMaterialCode Material code.
 */
void EffectKeeper::tryUpdateMaterial(const char* pMaterialCode) {
    if (isEqualString(mMaterialCode, pMaterialCode)) {
        return;
    }

    mMaterialCode = pMaterialCode;

    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->tryUpdateMaterial(mMaterialCode, mPrefixFlags);
    }

    mIsEmitted = true;
}

/**
 * Turns a material prefix on or off and updates the effect materials.
 * @param rType Prefix type.
 * @param isOn Whether the prefix is active.
 */
void EffectKeeper::updatePrefix(const EffectPrefixType& rType, bool isOn) {
    if (mPrefixFlags[rType] == isOn) {
        return;
    }

    mPrefixFlags[rType] = isOn;

    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->tryUpdateMaterial(mMaterialCode, mPrefixFlags);
    }
}

/**
 * Emits an effect at its current position.
 * @param pName Name of the effect.
 */
void EffectKeeper::emitEffectCurrentPos(const char* pName) {
    sead::Graphics::instance()->lockDrawContext();
    mIsEmitted = true;
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->emitEmitters(nullptr, true);
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Finds an effect by name.
 * @param pName Name of the effect.
 * @return The effect, or nullptr if there is none.
 */
Effect* EffectKeeper::findEffect(const char* pName) const {
    for (s32 i = 0; i < mEffectNum; i++) {
        if (isEqualString(pName, mEffects[i]->getName())) {
            return mEffects[i];
        }
    }

    return nullptr;
}

/**
 * Emits an effect.
 * @param pName Name of the effect.
 * @param pPos Emit position.
 */
void EffectKeeper::emitEffect(const char* pName, const sead::Vector3f* pPos) {
    sead::Graphics::instance()->lockDrawContext();
    mIsEmitted = true;
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->emitEmitters(pPos, false);
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Emits an effect unless it is already emitting.
 * @param pName Name of the effect.
 * @param pPos Emit position.
 * @return Whether the effect was emitted.
 */
bool EffectKeeper::tryEmitEffect(const char* pName, const sead::Vector3f* pPos) {
    sead::Graphics::instance()->lockDrawContext();
    mIsEmitted = true;
    Effect* effect = findEffect(pName);
    bool isEmitted = false;

    if (effect != nullptr) {
        isEmitted = effect->tryEmitEmitters(pPos, false);
    }

    sead::Graphics::instance()->unlockDrawContext();
    return isEmitted;
}

/**
 * Deletes the emitters of an effect.
 * @param pName Name of the effect.
 */
void EffectKeeper::deleteEffect(const char* pName) {
    sead::Graphics::instance()->lockDrawContext();
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        if (effect->isLoopOrInfinity()) {
            effect->isEmitterActive();
        }

        effect->tryDeleteEmitters();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Deletes the emitters of an effect if it exists.
 * @param pName Name of the effect.
 */
void EffectKeeper::tryDeleteEffect(const char* pName) {
    sead::Graphics::instance()->lockDrawContext();
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->tryDeleteEmitters();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Kills the emitters and particles of an effect if it exists.
 * @param pName Name of the effect.
 */
void EffectKeeper::tryDeleteEffectAndParticle(const char* pName) {
    sead::Graphics::instance()->lockDrawContext();
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->tryKillEmitterAndParticleAll();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Kills the emitters and particles of every effect.
 */
void EffectKeeper::tryKillEmitterAndParticleAll() {
    sead::Graphics::instance()->lockDrawContext();

    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->tryKillEmitterAndParticleAll();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Deletes the emitters of every effect.
 */
void EffectKeeper::deleteEffectAll() {
    sead::Graphics::instance()->lockDrawContext();

    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->tryDeleteEmitters();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Deletes and clears the emitters of every effect.
 */
void EffectKeeper::deleteAndClearEffectAll() {
    sead::Graphics::instance()->lockDrawContext();

    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->deleteAndClearEmitter();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Resumes calculation and drawing of every effect.
 */
void EffectKeeper::onCalcAndDraw() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->setStopCalcAndDraw_CAFE(false);
    }

    mIsEmitted = true;
}

/**
 * Stops calculation and drawing of every effect.
 */
void EffectKeeper::offCalcAndDraw() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->setStopCalcAndDraw_CAFE(true);
    }
}

/**
 * Forces calculation and drawing of every effect on or off.
 * @param isStop Whether to stop.
 */
void EffectKeeper::forceSetStopCalcAndDraw(bool isStop) {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->forceSetStopCalcAndDraw(isStop);
    }
}

/**
 * Enables or disables drawing of every effect.
 * @param isEnable Whether drawing is enabled.
 */
void EffectKeeper::setEnableDraw(bool isEnable) {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffects[i]->setEnableDraw(isEnable);
    }
}

/**
 * Enables or disables drawing of the effects with the given name.
 * @param isEnable Whether drawing is enabled.
 * @param pName Name of the effect.
 */
void EffectKeeper::setEnableDraw(bool isEnable, const char* pName) {
    for (s32 i = 0; i < mEffectNum; i++) {
        Effect* effect = mEffects[i];
        if (std::strcmp(effect->getName(), pName) == 0) {
            effect->setEnableDraw(isEnable);
        }
    }
}

/**
 * Sets the emit ratio of an effect.
 * @param pName Name of the effect.
 * @param ratio Emit ratio.
 */
void EffectKeeper::setEmitRatio(const char* pName, f32 ratio) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setEmitRatio(ratio);
    }
}

/**
 * Sets the emitter scale of an effect.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void EffectKeeper::setEmitterScale(const char* pName, const sead::Vector3f& rScale) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setEmitterScale(rScale);
    }
}

/**
 * Sets the emitter and particle scale of an effect.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void EffectKeeper::setEmitterAllScale(const char* pName, const sead::Vector3f& rScale) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setEmitterAllScale(rScale);
    }
}

/**
 * Sets the emitter volume scale of an effect.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void EffectKeeper::setEmitterVolumeScale(const char* pName, const sead::Vector3f& rScale) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setEmitterVolumeScale(rScale);
    }
}

/**
 * Sets the uniform particle scale of an effect.
 * @param pName Name of the effect.
 * @param scale Scale.
 */
void EffectKeeper::setParticleScale(const char* pName, f32 scale) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setParticleScale(scale);
    }
}

/**
 * Sets the particle scale of an effect.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void EffectKeeper::setParticleScale(const char* pName, const sead::Vector3f& rScale) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setParticleScale(rScale);
    }
}

/**
 * Sets the particle alpha of an effect.
 * @param pName Name of the effect.
 * @param alpha Alpha.
 */
void EffectKeeper::setParticleAlpha(const char* pName, f32 alpha) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setParticleAlpha(alpha);
    }
}

/**
 * Sets the particle color of an effect.
 * @param pName Name of the effect.
 * @param rColor Color.
 */
void EffectKeeper::setParticleColor(const char* pName, const sead::Color4f& rColor) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setParticleColor(rColor);
    }
}

/**
 * Sets the particle life scale of an effect.
 * @param pName Name of the effect.
 * @param scale Life scale.
 */
void EffectKeeper::setParticleLifeScale(const char* pName, f32 scale) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setParticleLifeScale(scale);
    }
}

/**
 * Sets the two emitter colors of an effect.
 * @param pName Name of the effect.
 * @param rColor0 First color.
 * @param rColor1 Second color.
 */
void EffectKeeper::setEmitterColors(const char* pName, const sead::Color4f& rColor0,
                                    const sead::Color4f& rColor1) {
    Effect* effect = findEffect(pName);

    if (effect != nullptr) {
        effect->setEmitterColors(rColor0, rColor1);
    }
}

/**
 * Finds a named matrix.
 * @param pName Name of the matrix.
 * @return The matrix.
 */
const sead::Matrix34f* EffectKeeper::findMtxPtr(const char* pName) {
    return mMtxPtrHolder->findMtxPtr(pName);
}

/**
 * Finds an effect by name.
 * @param pName Name of the effect.
 * @return The effect, or nullptr if there is none.
 */
Effect* EffectKeeper::tryFindEffect(const char* pName) const {
    for (s32 i = 0; i < mEffectNum; i++) {
        if (isEqualString(pName, mEffects[i]->getName())) {
            return mEffects[i];
        }
    }

    return nullptr;
}

}  // namespace al
