#include "Library/LiveActor/HitReactionKeeper.hpp"

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/Controller/PadRumbleFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/LiveActor/Util/ActorSceneUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/PostProcessing/RadialBlurDirector.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"
#include "Project/OceanWave/OceanWaveUserInfo.hpp"

namespace al {
/**
 * Constructs a hit reaction with default settings.
 */
HitReactionInfo::HitReactionInfo() = default;

/**
 * Creates a hit reaction keeper if the actor has a hit reaction file.
 * @param pActor The actor.
 * @param pResource The resource holding the file.
 * @param pName The file suffix.
 * @return The keeper, or nullptr.
 */
HitReactionKeeper* HitReactionKeeper::tryCreate(LiveActor* pActor, const Resource* pResource,
                                                const char* pName) {
    StringTmp<128> fileName;
    tryGetActorInitFileName(&fileName, pResource, "ActorHitReactionCtrl", pName);
    if (!isExistResourceYaml(pResource, fileName.cstr(), nullptr)) {
        return nullptr;
    }
    return new HitReactionKeeper(pActor, pResource, pName);
}

/**
 * Creates a hit reaction keeper if the layout actor has a hit reaction file.
 * @param pActor The layout actor.
 * @param pResource The resource holding the file.
 * @param pName The file suffix.
 * @return The keeper, or nullptr.
 */
HitReactionKeeper* HitReactionKeeper::tryCreate(LayoutActor* pActor, const Resource* pResource,
                                                const char* pName) {
    StringTmp<128> fileName;
    tryGetLayoutActorInitFileName(&fileName, pResource, "ActorHitReactionCtrl", pName);
    if (!isExistResourceYaml(pResource, fileName.cstr(), nullptr)) {
        return nullptr;
    }
    return new HitReactionKeeper(pActor, pResource, pName);
}

/**
 * Starts a hit reaction.
 * @param pName The reaction name.
 * @param pPos The effect position, or nullptr.
 * @param pSensor1 The first sensor used to place the effect, or nullptr.
 * @param pSensor2 The second sensor used to place the effect, or nullptr.
 */
void HitReactionKeeper::start(const char* pName, const sead::Vector3f* pPos,
                              const HitSensor* pSensor1, const HitSensor* pSensor2) {
    for (s32 i = 0; i < mReactionNum; i++) {
        const HitReactionInfo& info = mReactionInfos[i];
        if (!isEqualString(info.mReactionName, pName)) {
            continue;
        }

        if (info.mEffectName) {
            if (pPos) {
                emitEffect(mActor, info.mEffectName, pPos);
            } else if (pSensor1 && pSensor2) {
                sead::Vector3f pos;
                calcPosBetweenSensors(&pos, pSensor1, pSensor2,
                                      info.mEffectPosOffsetBetweenSensors);
                emitEffect(mActor, info.mEffectName, &pos);
            } else {
                emitEffect(mActor, info.mEffectName, nullptr);
            }
        }

        if (info.mSeName) {
            startSe(mActor, info.mSeName);
        }

        if (info.mOceanWaveName) {
            startOceanWave(mActor, info.mOceanWaveName);
        }

        if (info.mPadRumbleName) {
            if (mActor) {
                LiveActor* actor = mActor;
                const s32* port = mPadRumblePort;
                if (isEqualString("自分で振動", info.mPadRumbleType)) {
                    alPadRumbleFunction::startPadRumble(actor, info.mPadRumbleName, *port, false);
                } else if (isEqualString("全員", info.mPadRumbleType)) {
                    s32 playerNum = getPlayerNumMax(actor);
                    for (s32 p = 0; p < playerNum; p++) {
                        if (isPlayerDead(actor, p)) {
                            continue;
                        }
                        alPadRumbleFunction::startPadRumble(actor, info.mPadRumbleName,
                                                            getPlayerPort(actor, p), false);
                    }
                } else if (isEqualString("距離制限", info.mPadRumbleType)) {
                    s32 playerNum = getPlayerNumMax(actor);
                    for (s32 p = 0; p < playerNum; p++) {
                        if (isPlayerDead(actor, p)) {
                            continue;
                        }
                        if ((getPlayerPos(actor, p) - getTrans(actor)).length() >
                            info.mPadRumbleDistance) {
                            continue;
                        }
                        alPadRumbleFunction::startPadRumble(actor, info.mPadRumbleName,
                                                            getPlayerPort(actor, p), false);
                    }
                }
            } else if (mLayoutActor) {
                const s32* port = mPadRumblePort;
                PadRumbleDirector* director = alPadRumbleFunction::getPadRumbleDirector(mLayoutActor);
                if (isEqualString("自分で振動", info.mPadRumbleType)) {
                    alPadRumbleFunction::startPadRumbleNo3D(director, info.mPadRumbleName, *port,
                                                            false);
                } else if (isEqualString("全員", info.mPadRumbleType) ||
                           isEqualString("距離制限", info.mPadRumbleType)) {
                    for (s32 p = 0; p < 4; p++) {
                        s32 controllerPort = getPlayerControllerPort(p);
                        if (controllerPort == -1) {
                            break;
                        }
                        alPadRumbleFunction::startPadRumbleNo3D(director, info.mPadRumbleName,
                                                                controllerPort, false);
                    }
                }
            }
        }

        if (info.mCameraShakeName) {
            LiveActor* actor = mActor;
            bool isShake = true;
            if (isEqualString("距離で振動", info.mCameraShakeType)) {
                s32 playerNum = getPlayerNumMax(actor);
                for (s32 p = 0; p < playerNum; p++) {
                    if (isPlayerDead(actor, p)) {
                        continue;
                    }
                    if ((getPlayerPos(actor, p) - getTrans(actor)).length() >
                        info.mCameraShakeDistance) {
                        isShake = false;
                        break;
                    }
                }
            }
            if (isShake) {
                if (actor->mActorSceneInfo && actor->mActorSceneInfo->cameraDirector) {
                    startCameraShakeByHitReaction(actor, info.mCameraShakeName, actor->getName(),
                                                  info.mReactionName, -1, 0);
                } else {
                    requestStartCameraShake(actor, info.mCameraShakeName);
                }
            }
        }

        if (info.mStopSceneFrame > 0) {
            stopScene(mActor, info.mStopSceneFrame, info.mIsStopSceneForHitEffect ? 2 : 0, false,
                      info.mIsStopScenePlayers);
        }

        if (info.mRadialBlurFrame > 0) {
            emitRadialBlur(mActor, getTrans(mActor), info.mRadialBlurRadiusBegin,
                           info.mRadialBlurRadiusEnd, info.mRadialBlurFrame, -1);
        }
        return;
    }
}

/**
 * Reads the hit reactions of an actor.
 * @param pActor The actor.
 * @param pResource The resource holding the file.
 * @param pName The file suffix.
 */
HitReactionKeeper::HitReactionKeeper(LiveActor* pActor, const Resource* pResource,
                                     const char* pName)
    : mActor(pActor) {
    ByamlIter iter;
    tryGetActorInitFileIter(&iter, pResource, "ActorHitReactionCtrl", pName);
    mReactionNum = iter.getSize();
    mReactionInfos = new HitReactionInfo[mReactionNum];
    for (s32 i = 0; i < mReactionNum; i++) {
        ByamlIter reactionIter;
        iter.tryGetIterByIndex(&reactionIter, i);
        HitReactionInfo& info = mReactionInfos[i];
        reactionIter.tryGetStringByKey(&info.mReactionName, "ReactionName");
        reactionIter.tryGetStringByKey(&info.mEffectName, "EffectName");
        reactionIter.tryGetStringByKey(&info.mSoundName, "SoundName");
        reactionIter.tryGetStringByKey(&info.mSeName, "SeName");
        reactionIter.tryGetStringByKey(&info.mOceanWaveName, "OceanWaveName");
        reactionIter.tryGetStringByKey(&info.mPadRumbleType, "PadRumbleType");
        reactionIter.tryGetFloatByKey(&info.mPadRumbleDistance, "PadRumbleDistance");
        reactionIter.tryGetStringByKey(&info.mPadRumbleName, "PadRumbleName");
        reactionIter.tryGetStringByKey(&info.mCameraShakeType, "CameraShakeType");
        reactionIter.tryGetFloatByKey(&info.mCameraShakeDistance, "CameraShakeDistance");
        reactionIter.tryGetStringByKey(&info.mCameraShakeName, "CameraShakeName");
        reactionIter.tryGetIntByKey(&info.mStopSceneFrame, "StopSceneFrame");
        reactionIter.tryGetBoolByKey(&info.mIsStopSceneForHitEffect, "IsStopSceneForHitEffect");
        reactionIter.tryGetBoolByKey(&info.mIsStopScenePlayers, "IsStopScenePlayers");
        reactionIter.tryGetFloatByKey(&info.mEffectPosOffsetBetweenSensors,
                                      "EffectPosOffsetBetweenSensors");
        reactionIter.tryGetIntByKey(&info.mRadialBlurFrame, "RadialBlurFrame");
        reactionIter.tryGetFloatByKey(&info.mRadialBlurRadiusBegin, "RadialBlurRadiusBegin");
        reactionIter.tryGetFloatByKey(&info.mRadialBlurRadiusEnd, "RadialBlurRadiusEnd");
    }
}

/**
 * Reads the hit reactions of a layout actor.
 * @param pActor The layout actor.
 * @param pResource The resource holding the file.
 * @param pName The file suffix.
 */
HitReactionKeeper::HitReactionKeeper(LayoutActor* pActor, const Resource* pResource,
                                     const char* pName)
    : mLayoutActor(pActor) {
    ByamlIter iter;
    tryGetLayoutActorInitFileIter(&iter, pResource, "ActorHitReactionCtrl", pName);
    mReactionNum = iter.getSize();
    mReactionInfos = new HitReactionInfo[mReactionNum];
    for (s32 i = 0; i < mReactionNum; i++) {
        ByamlIter reactionIter;
        iter.tryGetIterByIndex(&reactionIter, i);
        HitReactionInfo& info = mReactionInfos[i];
        reactionIter.tryGetStringByKey(&info.mReactionName, "ReactionName");
        reactionIter.tryGetStringByKey(&info.mEffectName, "EffectName");
        reactionIter.tryGetStringByKey(&info.mSoundName, "SoundName");
        reactionIter.tryGetStringByKey(&info.mSeName, "SeName");
        reactionIter.tryGetStringByKey(&info.mOceanWaveName, "OceanWaveName");
        reactionIter.tryGetStringByKey(&info.mPadRumbleType, "PadRumbleType");
        reactionIter.tryGetFloatByKey(&info.mPadRumbleDistance, "PadRumbleDistance");
        reactionIter.tryGetStringByKey(&info.mPadRumbleName, "PadRumbleName");
        reactionIter.tryGetStringByKey(&info.mCameraShakeType, "CameraShakeType");
        reactionIter.tryGetFloatByKey(&info.mCameraShakeDistance, "CameraShakeDistance");
        reactionIter.tryGetStringByKey(&info.mCameraShakeName, "CameraShakeName");
        reactionIter.tryGetIntByKey(&info.mStopSceneFrame, "StopSceneFrame");
        reactionIter.tryGetBoolByKey(&info.mIsStopSceneForHitEffect, "IsStopSceneForHitEffect");
        reactionIter.tryGetBoolByKey(&info.mIsStopScenePlayers, "IsStopScenePlayers");
        reactionIter.tryGetFloatByKey(&info.mEffectPosOffsetBetweenSensors,
                                      "EffectPosOffsetBetweenSensors");
        reactionIter.tryGetIntByKey(&info.mRadialBlurFrame, "RadialBlurFrame");
        reactionIter.tryGetFloatByKey(&info.mRadialBlurRadiusBegin, "RadialBlurRadiusBegin");
        reactionIter.tryGetFloatByKey(&info.mRadialBlurRadiusEnd, "RadialBlurRadiusEnd");
    }
}
}  // namespace al
