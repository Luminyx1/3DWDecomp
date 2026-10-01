#include "Library/Effect/EffectSystemInfo.hpp"

#include <ptcl/seadPtclSystem.h>

#include "Library/Effect/PtclSystem.hpp"
#include "Library/Math/MatrixPtrHolder.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"
#include "Project/Effect/Core/IUseEffectKeeper.hpp"
#include "Project/Effect/Effect.hpp"
#include "Project/Effect/EffectEmitter.hpp"
#include "Project/Effect/EffectInfo.hpp"

namespace al {

/**
 * Constructs empty effect system info.
 */
EffectSystemInfo::EffectSystemInfo() = default;

/**
 * Gets the effect system that owns the particle system.
 * @return The effect system.
 */
EffectSystem* EffectSystemInfo::getEffectSystem() const {
    return mPtclSystem->getEffectSystem();
}

/**
 * Emits an effect at its current position.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 */
void emitEffectCurrentPos(IUseEffectKeeper* pUser, const char* pName) {
    pUser->getEffectKeeper()->emitEffectCurrentPos(pName);
}

/**
 * Emits an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param pPos Emit position.
 */
void emitEffect(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f* pPos) {
    pUser->getEffectKeeper()->emitEffect(pName, pPos);
}

/**
 * Checks whether the effect keeper has an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @return Whether the effect exists.
 */
bool isEffectExist(IUseEffectKeeper* pUser, const char* pName) {
    return pUser->getEffectKeeper()->tryFindEffect(pName) != nullptr;
}

/**
 * Checks whether an effect is drawn in snapshot camera mode.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @return Whether the effect exists and uses snapshot camera mode.
 */
bool isEffectSnapshotCameraMode(IUseEffectKeeper* pUser, const char* pName) {
    if (pUser->getEffectKeeper()->tryFindEffect(pName) == nullptr) {
        return false;
    }

    return pUser->getEffectKeeper()->tryFindEffect(pName)->getEffectInfo()->mParam.mIsSnapshotCameraMode;
}

/**
 * Emits an effect unless it is already emitting.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param pPos Emit position.
 * @return Whether the effect was emitted.
 */
bool tryEmitEffect(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f* pPos) {
    return pUser->getEffectKeeper()->tryEmitEffect(pName, pPos);
}

/**
 * Deletes the emitters of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 */
void deleteEffect(IUseEffectKeeper* pUser, const char* pName) {
    pUser->getEffectKeeper()->deleteEffect(pName);
}

/**
 * Deletes the emitters of an effect if it exists.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 */
void tryDeleteEffect(IUseEffectKeeper* pUser, const char* pName) {
    pUser->getEffectKeeper()->tryDeleteEffect(pName);
}

/**
 * Kills the emitters and particles of an effect if it exists.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 */
void tryDeleteEffectAndParticle(IUseEffectKeeper* pUser, const char* pName) {
    pUser->getEffectKeeper()->tryDeleteEffectAndParticle(pName);
}

/**
 * Deletes the emitters of every effect.
 * @param pUser Effect keeper user.
 */
void deleteEffectAll(IUseEffectKeeper* pUser) {
    pUser->getEffectKeeper()->deleteEffectAll();
}

/**
 * Kills the emitters and particles of every effect.
 * @param pUser Effect keeper user.
 */
void tryKillEmitterAndParticleAll(IUseEffectKeeper* pUser) {
    pUser->getEffectKeeper()->tryKillEmitterAndParticleAll();
}

/**
 * Kills the emitters and particles of every effect.
 * @param pUser Effect keeper user.
 */
void tryDeleteEmitterAndParticleAll(IUseEffectKeeper* pUser) {
    pUser->getEffectKeeper()->tryKillEmitterAndParticleAll();
}

/**
 * Resumes calculation and drawing of every effect.
 * @param pUser Effect keeper user.
 */
void onCalcAndDrawEffect(IUseEffectKeeper* pUser) {
    pUser->getEffectKeeper()->onCalcAndDraw();
}

/**
 * Stops calculation and drawing of every effect.
 * @param pUser Effect keeper user.
 */
void offCalcAndDrawEffect(IUseEffectKeeper* pUser) {
    pUser->getEffectKeeper()->offCalcAndDraw();
}

/**
 * Updates the effects if the user has an effect keeper.
 * @param pUser Effect keeper user.
 */
void updateEffects(IUseEffectKeeper* pUser) {
    if (pUser->getEffectKeeper() == nullptr) {
        return;
    }

    pUser->getEffectKeeper()->update();
}

/**
 * Forces calculation and drawing of every effect on or off.
 * @param pUser Effect keeper user.
 * @param isStop Whether to stop.
 */
void forceSetStopCalcAndDraw(IUseEffectKeeper* pUser, bool isStop) {
    pUser->getEffectKeeper()->forceSetStopCalcAndDraw(isStop);
}

/**
 * Checks whether an effect has an active emitter.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @return Whether the effect is emitting.
 */
bool isEffectEmitting(const IUseEffectKeeper* pUser, const char* pName) {
    return pUser->getEffectKeeper()->findEffect(pName)->isEmitterActive();
}

/**
 * Checks whether all emitters of an effect are active.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @return Whether the effect is fully emitting.
 */
bool isEffectEmittingFully(const IUseEffectKeeper* pUser, const char* pName) {
    return pUser->getEffectKeeper()->findEffect(pName)->isEmitterActiveFully();
}

/**
 * Sets the emit ratio of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param ratio Emit ratio.
 */
void setEffectEmitRatio(IUseEffectKeeper* pUser, const char* pName, f32 ratio) {
    pUser->getEffectKeeper()->setEmitRatio(pName, ratio);
}

/**
 * Sets the emitter scale of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void setEffectEmitterScale(IUseEffectKeeper* pUser, const char* pName,
                           const sead::Vector3f& rScale) {
    pUser->getEffectKeeper()->setEmitterScale(pName, rScale);
}

/**
 * Sets the emitter and particle scale of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void setEffectAllScale(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f& rScale) {
    pUser->getEffectKeeper()->setEmitterAllScale(pName, rScale);
}

/**
 * Sets the emitter volume scale of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void setEffectEmitterVolumeScale(IUseEffectKeeper* pUser, const char* pName,
                                 const sead::Vector3f& rScale) {
    pUser->getEffectKeeper()->setEmitterVolumeScale(pName, rScale);
}

/**
 * Sets the uniform particle scale of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param scale Scale.
 */
void setEffectParticleScale(IUseEffectKeeper* pUser, const char* pName, f32 scale) {
    pUser->getEffectKeeper()->setParticleScale(pName, scale);
}

/**
 * Sets the particle scale of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param rScale Scale.
 */
void setEffectParticleScale(IUseEffectKeeper* pUser, const char* pName,
                            const sead::Vector3f& rScale) {
    pUser->getEffectKeeper()->setParticleScale(pName, rScale);
}

/**
 * Sets the particle alpha of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param alpha Alpha.
 */
void setEffectParticleAlpha(IUseEffectKeeper* pUser, const char* pName, f32 alpha) {
    pUser->getEffectKeeper()->setParticleAlpha(pName, alpha);
}

/**
 * Sets the particle color of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param rColor Color.
 */
void setEffectParticleColor(IUseEffectKeeper* pUser, const char* pName,
                            const sead::Color4f& rColor) {
    pUser->getEffectKeeper()->setParticleColor(pName, rColor);
}

/**
 * Sets the two emitter colors of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param rColor0 First color.
 * @param rColor1 Second color.
 */
void setEffectEmitterColors(IUseEffectKeeper* pUser, const char* pName,
                            const sead::Color4f& rColor0, const sead::Color4f& rColor1) {
    pUser->getEffectKeeper()->setEmitterColors(pName, rColor0, rColor1);
}

/**
 * Sets the particle life scale of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param scale Life scale.
 */
void setParticleLifeScale(IUseEffectKeeper* pUser, const char* pName, f32 scale) {
    pUser->getEffectKeeper()->setParticleLifeScale(pName, scale);
}

/**
 * Sets the directional velocity of every valid emitter set of an effect.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param vel Directional velocity.
 */
void setEffectParticleDirectionalVel(IUseEffectKeeper* pUser, const char* pName, f32 vel) {
    Effect* effect = pUser->getEffectKeeper()->findEffect(pName);
    s32 emitterNum = effect->getEmitterNum();

    for (s32 i = 0; i < emitterNum; i++) {
        auto* handle = effect->getEmitter(i)->getHandle();

        if (handle->IsValid()) {
            handle->GetEmitterSet()->SetDirectionalVel(vel);
        }
    }
}

/**
 * Makes an effect follow a position.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param pPos Position to follow.
 */
void setEffectFollowPosPtr(IUseEffectKeeper* pUser, const char* pName,
                           const sead::Vector3f* pPos) {
    pUser->getEffectKeeper()->findEffect(pName)->setPosPtr(pPos);
}

/**
 * Makes an effect follow a matrix if it exists.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @param pMtx Matrix to follow.
 */
void setEffectFollowMtxPtr(IUseEffectKeeper* pUser, const char* pName,
                           const sead::Matrix34f* pMtx) {
    Effect* effect = pUser->getEffectKeeper()->findEffect(pName);

    if (effect != nullptr) {
        effect->setMtxPtr(pMtx);
    }
}

/**
 * Sets a named matrix and rebinds the effects attached to it.
 * @param pUser Effect keeper user.
 * @param pName Name of the matrix.
 * @param pMtx Matrix.
 */
void setEffectNamedMtxPtr(IUseEffectKeeper* pUser, const char* pName,
                          const sead::Matrix34f* pMtx) {
    pUser->getEffectKeeper()->getMtxPtrHolder()->setMtxPtr(pName, pMtx);
    alEffectKeeperInitFunction::updateNamedMtxPtr(pUser->getEffectKeeper(), pName);
}

/**
 * Sets a named matrix if the user has an effect keeper.
 * @param pUser Effect keeper user.
 * @param pName Name of the matrix.
 * @param pMtx Matrix.
 */
void trySetEffectNamedMtxPtr(IUseEffectKeeper* pUser, const char* pName,
                             const sead::Matrix34f* pMtx) {
    if (pUser->getEffectKeeper() == nullptr) {
        return;
    }

    pUser->getEffectKeeper()->getMtxPtrHolder()->setMtxPtr(pName, pMtx);
    alEffectKeeperInitFunction::updateNamedMtxPtr(pUser->getEffectKeeper(), pName);
}

/**
 * Switches the effects to a new material code.
 * @param pUser Effect keeper user.
 * @param pMaterialCode Material code.
 */
void tryUpdateEffectMaterialCode(IUseEffectKeeper* pUser, const char* pMaterialCode) {
    pUser->getEffectKeeper()->tryUpdateMaterial(pMaterialCode);
}

/**
 * Clears the material code of the effects.
 * @param pUser Effect keeper user.
 */
void resetEffectMaterialCode(IUseEffectKeeper* pUser) {
    pUser->getEffectKeeper()->tryUpdateMaterial("");
}

/**
 * Turns the water material prefix on or off.
 * @param pUser Effect keeper user.
 * @param isOn Whether the prefix is active.
 */
void updateEffectMaterialWater(IUseEffectKeeper* pUser, bool isOn) {
    pUser->getEffectKeeper()->updatePrefix(EffectPrefixType::Water, isOn);
}

/**
 * Turns the route dokan material prefix on or off.
 * @param pUser Effect keeper user.
 * @param isOn Whether the prefix is active.
 */
void updateEffectMaterialRouteDokan(IUseEffectKeeper* pUser, bool isOn) {
    pUser->getEffectKeeper()->updatePrefix(EffectPrefixType::RouteDokan, isOn);
}

/**
 * Turns the wet material prefix on or off.
 * @param pUser Effect keeper user.
 * @param isOn Whether the prefix is active.
 */
void updateEffectMaterialWet(IUseEffectKeeper* pUser, bool isOn) {
    pUser->getEffectKeeper()->updatePrefix(EffectPrefixType::Wet, isOn);
}

/**
 * Turns the puddle material prefix on or off.
 * @param pUser Effect keeper user.
 * @param isOn Whether the prefix is active.
 */
void updateEffectMaterialPuddle(IUseEffectKeeper* pUser, bool isOn) {
    pUser->getEffectKeeper()->updatePrefix(EffectPrefixType::Shallow, isOn);
}

/**
 * Gets the position an effect follows.
 * @param pUser Effect keeper user.
 * @param pName Name of the effect.
 * @return The followed position.
 */
const sead::Vector3f* getEffectPosPtr(IUseEffectKeeper* pUser, const char* pName) {
    return pUser->getEffectKeeper()->findEffect(pName)->getPosPtr();
}

}  // namespace al
