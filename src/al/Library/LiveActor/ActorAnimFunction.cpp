#include "Library/LiveActor/Util/ActorAnimUtil.hpp"

#include <math/seadMathCalcCommon.h>
#include <nn/g3d/g3d_ResFile.h>
#include <nn/g3d/g3d_ResModel.h>

#include "Library/Anim/AnimPlayerSkl.hpp"
#include "Library/Anim/SklAnimRetargettingInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/Model/SimpleModelG3D.hpp"
#include "Project/Anim/AnimPlayerSimple.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
inline AnimPlayerSkl* getSkl(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelCafe->mAnimPlayerSkl;
}

inline AnimPlayerMat* getMtp(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelCafe->mAnimPlayerMat1;
}

inline AnimPlayerMat* getMts(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelCafe->mAnimPlayerMat2;
}

inline AnimPlayerMat* getMcl(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelCafe->mAnimPlayerMat0;
}

inline AnimPlayerVis* getVis(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelCafe->mAnimPlayerVis;
}
}  // namespace

/**
 * Starts a skeletal animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 */
void startSklAnim(LiveActor* pActor, const char* pAnimName) {
    getSkl(pActor)->startSklAnim(nullptr, pAnimName, nullptr, nullptr, nullptr, nullptr, nullptr);
}

/**
 * Starts a skeletal animation interpolated from another one.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param pInterpoleAnimName The animation to interpolate from.
 */
void startSklAnimInterpole(LiveActor* pActor, const char* pAnimName, const char* pInterpoleAnimName) {
    getSkl(pActor)->startSklAnim(pInterpoleAnimName, pAnimName, nullptr, nullptr, nullptr, nullptr, nullptr);
}

/**
 * Starts a skeletal animation if it exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartSklAnimIfExist(LiveActor* pActor, const char* pAnimName) {
    if (!isSklAnimExist(pActor, pAnimName)) {
        return false;
    }

    startSklAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a skeletal animation exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isSklAnimExist(const LiveActor* pActor, const char* pAnimName) {
    return isSklAnimExist(pActor) && getSkl(pActor)->isSklAnimExist(pAnimName);
}

/**
 * Starts a skeletal animation unless it is already playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartSklAnimIfNotPlaying(LiveActor* pActor, const char* pAnimName) {
    if (isSklAnimPlaying(pActor, pAnimName, 0)) {
        return false;
    }

    startSklAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a skeletal animation is playing in a blend slot.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param index The blend index.
 * @return Whether the condition holds.
 */
bool isSklAnimPlaying(const LiveActor* pActor, const char* pAnimName, s32 index) {
    const char* playingName = getPlayingSklAnimName(pActor, index);
    return playingName && isEqualString(pAnimName, playingName);
}

/**
 * Starts a blend of up to six skeletal animations.
 * @param pActor The actor.
 * @param pAnimName0 Animation 0.
 * @param pAnimName1 Animation 1.
 * @param pAnimName2 Animation 2.
 * @param pAnimName3 Animation 3.
 * @param pAnimName4 Animation 4.
 * @param pAnimName5 Animation 5.
 */
void startSklAnimBlend(LiveActor* pActor, const char* pAnimName0, const char* pAnimName1, const char* pAnimName2, const char* pAnimName3, const char* pAnimName4, const char* pAnimName5) {
    getSkl(pActor)->startSklAnim(nullptr, pAnimName0, pAnimName1, pAnimName2, pAnimName3, pAnimName4, pAnimName5);
}

/**
 * Starts an interpolated blend of up to six skeletal animations.
 * @param pActor The actor.
 * @param pInterpoleAnimName The animation to interpolate from.
 * @param pAnimName0 Animation 0.
 * @param pAnimName1 Animation 1.
 * @param pAnimName2 Animation 2.
 * @param pAnimName3 Animation 3.
 * @param pAnimName4 Animation 4.
 * @param pAnimName5 Animation 5.
 */
void startSklAnimBlendInterpole(LiveActor* pActor, const char* pInterpoleAnimName, const char* pAnimName0, const char* pAnimName1, const char* pAnimName2, const char* pAnimName3, const char* pAnimName4, const char* pAnimName5) {
    getSkl(pActor)->startSklAnim(pInterpoleAnimName, pAnimName0, pAnimName1, pAnimName2, pAnimName3, pAnimName4, pAnimName5);
}

/**
 * Sets the frame of a skeletal animation without updating the pose.
 * @param pActor The actor.
 * @param frame The frame.
 * @param index The blend index.
 */
void setSklAnimFrameNoUpdate(LiveActor* pActor, f32 frame, s32 index) {
    getSkl(pActor)->setSklAnimFrame(index, frame, false);
}

/**
 * Copies the playing skeletal animation state of another actor.
 * @param pActor The actor.
 * @param pSrcActor The actor to copy from.
 */
void copySklAnim(LiveActor* pActor, const LiveActor* pSrcActor) {
    s32 blendNum = sead::Mathi::min(getSkl(pSrcActor)->getSklAnimBlendNum(),
                                    getSkl(pActor)->getSklAnimBlendNum());
    for (s32 i = 0; i < blendNum; i++) {
        if (getSkl(pActor)->isSklAnimPlaying(i) && getSkl(pSrcActor)->isSklAnimPlaying(i)) {
            f32 frame = getSkl(pSrcActor)->getSklAnimFrame(i);
            getSkl(pActor)->setSklAnimFrame(i, frame, true);
        }
    }

    blendNum = sead::Mathi::min(getSkl(pSrcActor)->getSklAnimBlendNum(),
                                getSkl(pActor)->getSklAnimBlendNum());
    for (s32 i = 0; i < blendNum; i++) {
        if (getSkl(pActor)->isSklAnimPlaying(i)) {
            f32 frameRate = getSkl(pSrcActor)->getSklAnimFrameRate(i);
            getSkl(pActor)->setSklAnimFrameRate(i, frameRate);
        }
    }

    if (getSkl(pActor)->getSklAnimBlendNum() > 1 && getSkl(pSrcActor)->getSklAnimBlendNum() > 1) {
        const AnimPlayerSkl* srcSkl = getSkl(pSrcActor);

        for (s32 i = 0; i < srcSkl->getSklAnimBlendNum(); i++) {
            if (getSkl(pActor)->isSklAnimPlaying(i) && getSkl(pSrcActor)->isSklAnimPlaying(i)) {
                f32 weight = getSkl(pSrcActor)->getSklAnimBlendWeight(i);
                getSkl(pActor)->setSklAnimBlendWeight(i, weight);
            }
        }
    }

    if (isExistSklAnimRetargetting(pActor) && isExistSklAnimRetargetting(pSrcActor)) {
        if (isSklAnimRetargettingValid(pSrcActor)) {
            validateSklAnimRetargetting(pActor);
        } else {
            invalidateSklAnimRetargetting(pActor);
        }
    }
}

/**
 * Checks whether skeletal animation retargetting is bound.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isExistSklAnimRetargetting(const LiveActor* pActor) {
    return getSkl(pActor)->mRetargettingInfo != nullptr;
}

/**
 * Checks whether skeletal animation retargetting is enabled.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isSklAnimRetargettingValid(const LiveActor* pActor) {
    return getSkl(pActor)->mIsRetargettingValid;
}

/**
 * Enables skeletal animation retargetting.
 * @param pActor The actor.
 */
void validateSklAnimRetargetting(const LiveActor* pActor) {
    getSkl(pActor)->mIsRetargettingValid = true;
}

/**
 * Disables skeletal animation retargetting.
 * @param pActor The actor.
 */
void invalidateSklAnimRetargetting(const LiveActor* pActor) {
    getSkl(pActor)->mIsRetargettingValid = false;
}

/**
 * Resets skeletal animation interpolation.
 * @param pActor The actor.
 */
void clearSklAnimInterpole(LiveActor* pActor) {
    getSkl(pActor)->reset();
}

/**
 * Checks whether the actor has a skeletal animation player.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isSklAnimExist(const LiveActor* pActor) {
    return getSkl(pActor) != nullptr;
}

/**
 * Checks whether a skeletal animation ended.
 * @param pActor The actor.
 * @param index The blend index.
 * @return Whether the condition holds.
 */
bool isSklAnimEnd(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->isSklAnimEnd(index);
}

/**
 * Checks whether a skeletal animation plays only once.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isSklAnimOneTime(const LiveActor* pActor, const char* pAnimName) {
    return getSkl(pActor)->isSklAnimOneTime(pAnimName);
}

/**
 * Checks whether the skeletal animation in a blend slot plays only once.
 * @param pActor The actor.
 * @param index The blend index.
 * @return Whether the condition holds.
 */
bool isSklAnimOneTime(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->isSklAnimOneTime(index);
}

/**
 * Checks whether a blend slot plays a skeletal animation.
 * @param pActor The actor.
 * @param index The blend index.
 * @return Whether the condition holds.
 */
bool isSklAnimPlaying(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->isSklAnimPlaying(index);
}

/**
 * Gets the skeletal animation playing in a blend slot.
 * @param pActor The actor.
 * @param index The blend index.
 * @return The animation name.
 */
const char* getPlayingSklAnimName(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->getPlayingSklAnimName(index);
}

/**
 * Gets the frame of a skeletal animation.
 * @param pActor The actor.
 * @param index The blend index.
 * @return The value.
 */
f32 getSklAnimFrame(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->getSklAnimFrame(index);
}

/**
 * Gets the frame rate of a skeletal animation.
 * @param pActor The actor.
 * @param index The blend index.
 * @return The value.
 */
f32 getSklAnimFrameRate(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->getSklAnimFrameRate(index);
}

/**
 * Gets the last frame of a skeletal animation.
 * @param pActor The actor.
 * @param index The blend index.
 * @return The value.
 */
f32 getSklAnimFrameMax(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->getSklAnimFrameMax(index);
}

/**
 * Gets the last frame of a skeletal animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return The value.
 */
f32 getSklAnimFrameMax(const LiveActor* pActor, const char* pAnimName) {
    return getSkl(pActor)->getSklAnimFrameMax(pAnimName);
}

/**
 * Sets the frame of a skeletal animation.
 * @param pActor The actor.
 * @param frame The frame.
 * @param index The blend index.
 */
void setSklAnimFrame(LiveActor* pActor, f32 frame, s32 index) {
    getSkl(pActor)->setSklAnimFrame(index, frame, true);
}

/**
 * Sets the frame rate of a skeletal animation.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 * @param index The blend index.
 */
void setSklAnimFrameRate(LiveActor* pActor, f32 frameRate, s32 index) {
    getSkl(pActor)->setSklAnimFrameRate(index, frameRate);
}

/**
 * Gets the weight of a blend slot.
 * @param pActor The actor.
 * @param index The blend index.
 * @return The value.
 */
f32 getSklAnimBlendWeight(const LiveActor* pActor, s32 index) {
    return getSkl(pActor)->getSklAnimBlendWeight(index);
}

/**
 * Sets the weight of a blend slot.
 * @param pActor The actor.
 * @param weight The weight.
 * @param index The blend index.
 */
void setSklAnimBlendWeight(LiveActor* pActor, f32 weight, s32 index) {
    getSkl(pActor)->setSklAnimBlendWeight(index, weight);
}

/**
 * Sets the weights of two blend slots so they add up to one.
 * @param pActor The actor.
 * @param weight The weight of the first slot.
 */
void setSklAnimBlendWeightDouble(LiveActor* pActor, f32 weight) {
    setSklAnimBlendWeight(pActor, weight, 0);
    setSklAnimBlendWeight(pActor, 1.0f - weight, 1);
}

/**
 * Sets the weights of the first 2 blend slots.
 * @param pActor The actor.
 * @param weight0 The weight of slot 0.
 * @param weight1 The weight of slot 1.
 */
void setSklAnimBlendWeightDouble(LiveActor* pActor, f32 weight0, f32 weight1) {
    setSklAnimBlendWeight(pActor, weight0, 0);
    setSklAnimBlendWeight(pActor, weight1, 1);
}

/**
 * Sets the weights of the first 3 blend slots.
 * @param pActor The actor.
 * @param weight0 The weight of slot 0.
 * @param weight1 The weight of slot 1.
 * @param weight2 The weight of slot 2.
 */
void setSklAnimBlendWeightTriple(LiveActor* pActor, f32 weight0, f32 weight1, f32 weight2) {
    setSklAnimBlendWeight(pActor, weight0, 0);
    setSklAnimBlendWeight(pActor, weight1, 1);
    setSklAnimBlendWeight(pActor, weight2, 2);
}

/**
 * Sets the weights of the first 4 blend slots.
 * @param pActor The actor.
 * @param weight0 The weight of slot 0.
 * @param weight1 The weight of slot 1.
 * @param weight2 The weight of slot 2.
 * @param weight3 The weight of slot 3.
 */
void setSklAnimBlendWeightQuad(LiveActor* pActor, f32 weight0, f32 weight1, f32 weight2, f32 weight3) {
    setSklAnimBlendWeight(pActor, weight0, 0);
    setSklAnimBlendWeight(pActor, weight1, 1);
    setSklAnimBlendWeight(pActor, weight2, 2);
    setSklAnimBlendWeight(pActor, weight3, 3);
}

/**
 * Sets the weights of the first 5 blend slots.
 * @param pActor The actor.
 * @param weight0 The weight of slot 0.
 * @param weight1 The weight of slot 1.
 * @param weight2 The weight of slot 2.
 * @param weight3 The weight of slot 3.
 * @param weight4 The weight of slot 4.
 */
void setSklAnimBlendWeightFivefold(LiveActor* pActor, f32 weight0, f32 weight1, f32 weight2, f32 weight3, f32 weight4) {
    setSklAnimBlendWeight(pActor, weight0, 0);
    setSklAnimBlendWeight(pActor, weight1, 1);
    setSklAnimBlendWeight(pActor, weight2, 2);
    setSklAnimBlendWeight(pActor, weight3, 3);
    setSklAnimBlendWeight(pActor, weight4, 4);
}

/**
 * Sets the weights of the first 6 blend slots.
 * @param pActor The actor.
 * @param weight0 The weight of slot 0.
 * @param weight1 The weight of slot 1.
 * @param weight2 The weight of slot 2.
 * @param weight3 The weight of slot 3.
 * @param weight4 The weight of slot 4.
 * @param weight5 The weight of slot 5.
 */
void setSklAnimBlendWeightSixfold(LiveActor* pActor, f32 weight0, f32 weight1, f32 weight2, f32 weight3, f32 weight4, f32 weight5) {
    setSklAnimBlendWeight(pActor, weight0, 0);
    setSklAnimBlendWeight(pActor, weight1, 1);
    setSklAnimBlendWeight(pActor, weight2, 2);
    setSklAnimBlendWeight(pActor, weight3, 3);
    setSklAnimBlendWeight(pActor, weight4, 4);
    setSklAnimBlendWeight(pActor, weight5, 5);
}

/**
 * Sets the frame of all playing blend slots.
 * @param pActor The actor.
 * @param frame The frame of the first slot.
 * @param isSync Whether to scale the other slots by their length.
 */
void setSklAnimBlendFrameAll(LiveActor* pActor, f32 frame, bool isSync) {
    AnimPlayerSkl* skl = getSkl(pActor);
    skl->setSklAnimFrame(0, frame, true);

    if (isSync) {
        f32 frameMax = skl->getSklAnimFrameMax(0);

        for (s32 i = 1; i < skl->getSklAnimBlendNum(); i++) {
            if (getSkl(pActor)->isSklAnimPlaying(i)) {
                f32 value = frame / frameMax * skl->getSklAnimFrameMax(i);
                getSkl(pActor)->setSklAnimFrame(i, value, true);
            }
        }
    } else {
        for (s32 i = 1; i < skl->getSklAnimBlendNum(); i++) {
            if (getSkl(pActor)->isSklAnimPlaying(i)) {
                getSkl(pActor)->setSklAnimFrame(i, frame, true);
            }
        }
    }
}

/**
 * Sets the frame rate of all playing blend slots.
 * @param pActor The actor.
 * @param frameRate The frame rate of the first slot.
 * @param isSync Whether to scale the other slots by their length.
 */
void setSklAnimBlendFrameRateAll(LiveActor* pActor, f32 frameRate, bool isSync) {
    AnimPlayerSkl* skl = getSkl(pActor);
    skl->setSklAnimFrameRate(0, frameRate);

    if (isSync) {
        f32 frameMax = skl->getSklAnimFrameMax(0);

        for (s32 i = 1; i < skl->getSklAnimBlendNum(); i++) {
            if (getSkl(pActor)->isSklAnimPlaying(i)) {
                f32 value = frameRate / frameMax * skl->getSklAnimFrameMax(i);
                getSkl(pActor)->setSklAnimFrameRate(i, value);
            }
        }
    } else {
        for (s32 i = 1; i < skl->getSklAnimBlendNum(); i++) {
            if (getSkl(pActor)->isSklAnimPlaying(i)) {
                getSkl(pActor)->setSklAnimFrameRate(i, frameRate);
            }
        }
    }
}

/**
 * Starts a texture pattern animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 */
void startMtpAnim(LiveActor* pActor, const char* pAnimName) {
    getMtp(pActor)->startAnim(pAnimName);
}

/**
 * Starts a texture pattern animation and stops it at a frame.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param frame The frame.
 */
void startMtpAnimAndSetFrameAndStop(LiveActor* pActor, const char* pAnimName, f32 frame) {
    startMtpAnim(pActor, pAnimName);
    setMtpAnimFrame(pActor, frame);
    setMtpAnimFrameRate(pActor, 0.0f);
}

/**
 * Sets the frame of the texture pattern animation.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setMtpAnimFrame(const LiveActor* pActor, f32 frame) {
    getMtp(pActor)->setAnimFrame(frame);
}

/**
 * Sets the frame rate of the texture pattern animation.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 */
void setMtpAnimFrameRate(const LiveActor* pActor, f32 frameRate) {
    getMtp(pActor)->setAnimFrameRate(frameRate);
}

/**
 * Starts a texture pattern animation if it exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartMtpAnimIfExist(LiveActor* pActor, const char* pAnimName) {
    if (!isMtpAnimExist(pActor, pAnimName)) {
        return false;
    }

    startMtpAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a texture pattern animation exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMtpAnimExist(const LiveActor* pActor, const char* pAnimName) {
    return isMtpAnimExist(pActor) && getMtp(pActor)->isAnimExist(pAnimName);
}

/**
 * Starts a texture pattern animation unless it is already playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartMtpAnimIfNotPlaying(LiveActor* pActor, const char* pAnimName) {
    if (isMtpAnimPlaying(pActor, pAnimName)) {
        return false;
    }

    startMtpAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a texture pattern animation is playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMtpAnimPlaying(const LiveActor* pActor, const char* pAnimName) {
    return isEqualString(pAnimName, getPlayingMtpAnimName(pActor));
}

/**
 * Clears the texture pattern animation.
 * @param pActor The actor.
 */
void clearMtpAnim(LiveActor* pActor) {
    getMtp(pActor)->clearAnim();
}

/**
 * Checks whether the actor has a texture pattern animation player.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtpAnimExist(const LiveActor* pActor) {
    return getMtp(pActor) != nullptr;
}

/**
 * Checks whether the texture pattern animation ended.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtpAnimEnd(const LiveActor* pActor) {
    return getMtp(pActor)->isAnimEnd();
}

/**
 * Checks whether a texture pattern animation plays only once.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMtpAnimOneTime(const LiveActor* pActor, const char* pAnimName) {
    return getMtp(pActor)->isAnimOneTime(pAnimName);
}

/**
 * Checks whether the current texture pattern animation plays only once.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtpAnimOneTime(const LiveActor* pActor) {
    return getMtp(pActor)->isAnimOneTime();
}

/**
 * Gets the playing texture pattern animation.
 * @param pActor The actor.
 * @return The animation name.
 */
const char* getPlayingMtpAnimName(const LiveActor* pActor) {
    return getMtp(pActor)->getPlayingAnimName();
}

/**
 * Checks whether a texture pattern animation is playing.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtpAnimPlaying(const LiveActor* pActor) {
    return isMtpAnimExist(pActor) && getMtp(pActor)->isAnimPlaying();
}

/**
 * Gets the frame of the texture pattern animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMtpAnimFrame(const LiveActor* pActor) {
    return getMtp(pActor)->getAnimFrame();
}

/**
 * Gets the frame rate of the texture pattern animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMtpAnimFrameRate(const LiveActor* pActor) {
    return getMtp(pActor)->getAnimFrameRate();
}

/**
 * Gets the last frame of the texture pattern animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMtpAnimFrameMax(const LiveActor* pActor) {
    return getMtp(pActor)->getAnimFrameMax();
}

/**
 * Gets the last frame of a texture pattern animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return The value.
 */
f32 getMtpAnimFrameMax(const LiveActor* pActor, const char* pAnimName) {
    return getMtp(pActor)->getAnimFrameMax(pAnimName);
}

/**
 * Stops the texture pattern animation at a frame.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setMtpAnimFrameAndStop(LiveActor* pActor, f32 frame) {
    setMtpAnimFrame(pActor, frame);
    setMtpAnimFrameRate(pActor, 0.0f);
}

/**
 * Stops the texture pattern animation at its last frame.
 * @param pActor The actor.
 */
void setMtpAnimFrameAndStopEnd(LiveActor* pActor) {
    setMtpAnimFrameAndStop(pActor, getMtpAnimFrameMax(pActor));
}

/**
 * Starts a color animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 */
void startMclAnim(LiveActor* pActor, const char* pAnimName) {
    getMcl(pActor)->startAnim(pAnimName);
}

/**
 * Starts a color animation and stops it at a frame.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param frame The frame.
 */
void startMclAnimAndSetFrameAndStop(LiveActor* pActor, const char* pAnimName, f32 frame) {
    startMclAnim(pActor, pAnimName);
    setMclAnimFrame(pActor, frame);
    setMclAnimFrameRate(pActor, 0.0f);
}

/**
 * Sets the frame of the color animation.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setMclAnimFrame(const LiveActor* pActor, f32 frame) {
    getMcl(pActor)->setAnimFrame(frame);
}

/**
 * Sets the frame rate of the color animation.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 */
void setMclAnimFrameRate(const LiveActor* pActor, f32 frameRate) {
    getMcl(pActor)->setAnimFrameRate(frameRate);
}

/**
 * Starts a color animation if it exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartMclAnimIfExist(LiveActor* pActor, const char* pAnimName) {
    if (!isMclAnimExist(pActor, pAnimName)) {
        return false;
    }

    startMclAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a color animation exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMclAnimExist(const LiveActor* pActor, const char* pAnimName) {
    return isMclAnimExist(pActor) && getMcl(pActor)->isAnimExist(pAnimName);
}

/**
 * Starts a color animation unless it is already playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartMclAnimIfNotPlaying(LiveActor* pActor, const char* pAnimName) {
    if (isMclAnimPlaying(pActor, pAnimName)) {
        return false;
    }

    startMclAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a color animation is playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMclAnimPlaying(const LiveActor* pActor, const char* pAnimName) {
    return isEqualString(pAnimName, getPlayingMclAnimName(pActor));
}

/**
 * Clears the color animation.
 * @param pActor The actor.
 */
void clearMclAnim(LiveActor* pActor) {
    getMcl(pActor)->clearAnim();
}

/**
 * Checks whether the actor has a color animation player.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMclAnimExist(const LiveActor* pActor) {
    return getMcl(pActor) != nullptr;
}

/**
 * Checks whether the color animation ended.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMclAnimEnd(const LiveActor* pActor) {
    return getMcl(pActor)->isAnimEnd();
}

/**
 * Checks whether a color animation plays only once.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMclAnimOneTime(const LiveActor* pActor, const char* pAnimName) {
    return getMcl(pActor)->isAnimOneTime(pAnimName);
}

/**
 * Checks whether the current color animation plays only once.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMclAnimOneTime(const LiveActor* pActor) {
    return getMcl(pActor)->isAnimOneTime();
}

/**
 * Gets the playing color animation.
 * @param pActor The actor.
 * @return The animation name.
 */
const char* getPlayingMclAnimName(const LiveActor* pActor) {
    return getMcl(pActor)->getPlayingAnimName();
}

/**
 * Checks whether a color animation is playing.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMclAnimPlaying(const LiveActor* pActor) {
    return isMclAnimExist(pActor) && getMcl(pActor)->isAnimPlaying();
}

/**
 * Gets the frame of the color animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMclAnimFrame(const LiveActor* pActor) {
    return getMcl(pActor)->getAnimFrame();
}

/**
 * Gets the frame rate of the color animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMclAnimFrameRate(const LiveActor* pActor) {
    return getMcl(pActor)->getAnimFrameRate();
}

/**
 * Gets the last frame of the color animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMclAnimFrameMax(const LiveActor* pActor) {
    return getMcl(pActor)->getAnimFrameMax();
}

/**
 * Gets the last frame of a color animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return The value.
 */
f32 getMclAnimFrameMax(const LiveActor* pActor, const char* pAnimName) {
    return getMcl(pActor)->getAnimFrameMax(pAnimName);
}

/**
 * Stops the color animation at a frame.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setMclAnimFrameAndStop(LiveActor* pActor, f32 frame) {
    setMclAnimFrame(pActor, frame);
    setMclAnimFrameRate(pActor, 0.0f);
}

/**
 * Stops the color animation at its last frame.
 * @param pActor The actor.
 */
void setMclAnimFrameAndStopEnd(LiveActor* pActor) {
    setMclAnimFrameAndStop(pActor, getMclAnimFrameMax(pActor));
}

/**
 * Starts a texture SRT animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 */
void startMtsAnim(LiveActor* pActor, const char* pAnimName) {
    getMts(pActor)->startAnim(pAnimName);
}

/**
 * Starts a texture SRT animation and stops it at a frame.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param frame The frame.
 */
void startMtsAnimAndSetFrameAndStop(LiveActor* pActor, const char* pAnimName, f32 frame) {
    startMtsAnim(pActor, pAnimName);
    setMtsAnimFrame(pActor, frame);
    setMtsAnimFrameRate(pActor, 0.0f);
}

/**
 * Sets the frame of the texture SRT animation.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setMtsAnimFrame(const LiveActor* pActor, f32 frame) {
    getMts(pActor)->setAnimFrame(frame);
}

/**
 * Sets the frame rate of the texture SRT animation.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 */
void setMtsAnimFrameRate(const LiveActor* pActor, f32 frameRate) {
    getMts(pActor)->setAnimFrameRate(frameRate);
}

/**
 * Starts a texture SRT animation if it exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartMtsAnimIfExist(LiveActor* pActor, const char* pAnimName) {
    if (!isMtsAnimExist(pActor, pAnimName)) {
        return false;
    }

    startMtsAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a texture SRT animation exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMtsAnimExist(const LiveActor* pActor, const char* pAnimName) {
    return isMtsAnimExist(pActor) && getMts(pActor)->isAnimExist(pAnimName);
}

/**
 * Starts a texture SRT animation unless it is already playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartMtsAnimIfNotPlaying(LiveActor* pActor, const char* pAnimName) {
    if (isMtsAnimPlaying(pActor, pAnimName)) {
        return false;
    }

    startMtsAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a texture SRT animation is playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMtsAnimPlaying(const LiveActor* pActor, const char* pAnimName) {
    return isEqualString(pAnimName, getPlayingMtsAnimName(pActor));
}

/**
 * Clears the texture SRT animation.
 * @param pActor The actor.
 */
void clearMtsAnim(LiveActor* pActor) {
    getMts(pActor)->clearAnim();
}

/**
 * Checks whether the actor has a texture SRT animation player.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtsAnimExist(const LiveActor* pActor) {
    return getMts(pActor) != nullptr;
}

/**
 * Checks whether the texture SRT animation ended.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtsAnimEnd(const LiveActor* pActor) {
    return getMts(pActor)->isAnimEnd();
}

/**
 * Checks whether a texture SRT animation plays only once.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isMtsAnimOneTime(const LiveActor* pActor, const char* pAnimName) {
    return getMts(pActor)->isAnimOneTime(pAnimName);
}

/**
 * Checks whether the current texture SRT animation plays only once.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtsAnimOneTime(const LiveActor* pActor) {
    return getMts(pActor)->isAnimOneTime();
}

/**
 * Gets the playing texture SRT animation.
 * @param pActor The actor.
 * @return The animation name.
 */
const char* getPlayingMtsAnimName(const LiveActor* pActor) {
    return getMts(pActor)->getPlayingAnimName();
}

/**
 * Checks whether a texture SRT animation is playing.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isMtsAnimPlaying(const LiveActor* pActor) {
    return isMtsAnimExist(pActor) && getMts(pActor)->isAnimPlaying();
}

/**
 * Gets the frame of the texture SRT animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMtsAnimFrame(const LiveActor* pActor) {
    return getMts(pActor)->getAnimFrame();
}

/**
 * Gets the frame rate of the texture SRT animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMtsAnimFrameRate(const LiveActor* pActor) {
    return getMts(pActor)->getAnimFrameRate();
}

/**
 * Gets the last frame of the texture SRT animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getMtsAnimFrameMax(const LiveActor* pActor) {
    return getMts(pActor)->getAnimFrameMax();
}

/**
 * Gets the last frame of a texture SRT animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return The value.
 */
f32 getMtsAnimFrameMax(const LiveActor* pActor, const char* pAnimName) {
    return getMts(pActor)->getAnimFrameMax(pAnimName);
}

/**
 * Stops the texture SRT animation at a frame.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setMtsAnimFrameAndStop(LiveActor* pActor, f32 frame) {
    setMtsAnimFrame(pActor, frame);
    setMtsAnimFrameRate(pActor, 0.0f);
}

/**
 * Stops the texture SRT animation at its last frame.
 * @param pActor The actor.
 */
void setMtsAnimFrameAndStopEnd(LiveActor* pActor) {
    setMtsAnimFrameAndStop(pActor, getMtsAnimFrameMax(pActor));
}

/**
 * Starts a visibility animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 */
void startVisAnim(LiveActor* pActor, const char* pAnimName) {
    getVis(pActor)->startAnim(pAnimName);
}

/**
 * Starts a visibility animation and stops it at a frame.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param frame The frame.
 */
void startVisAnimAndSetFrameAndStop(LiveActor* pActor, const char* pAnimName, f32 frame) {
    startVisAnim(pActor, pAnimName);
    setVisAnimFrame(pActor, frame);
    setVisAnimFrameRate(pActor, 0.0f);
}

/**
 * Sets the frame of the visibility animation.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setVisAnimFrame(const LiveActor* pActor, f32 frame) {
    getVis(pActor)->setAnimFrame(frame);
}

/**
 * Sets the frame rate of the visibility animation.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 */
void setVisAnimFrameRate(const LiveActor* pActor, f32 frameRate) {
    getVis(pActor)->setAnimFrameRate(frameRate);
}

/**
 * Starts a visibility animation if it exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartVisAnimIfExist(LiveActor* pActor, const char* pAnimName) {
    if (!isVisAnimExist(pActor, pAnimName)) {
        return false;
    }

    startVisAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a visibility animation exists.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isVisAnimExist(const LiveActor* pActor, const char* pAnimName) {
    return isVisAnimExist(pActor) && getVis(pActor)->isAnimExist(pAnimName);
}

/**
 * Starts a visibility animation unless it is already playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool tryStartVisAnimIfNotPlaying(LiveActor* pActor, const char* pAnimName) {
    if (isVisAnimPlaying(pActor, pAnimName)) {
        return false;
    }

    startVisAnim(pActor, pAnimName);
    return true;
}

/**
 * Checks whether a visibility animation is playing.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isVisAnimPlaying(const LiveActor* pActor, const char* pAnimName) {
    return isEqualString(pAnimName, getPlayingVisAnimName(pActor));
}

/**
 * Clears the visibility animation.
 * @param pActor The actor.
 */
void clearVisAnim(LiveActor* pActor) {
    getVis(pActor)->clearAnim();
}

/**
 * Checks whether the actor has a visibility animation player.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isVisAnimExist(const LiveActor* pActor) {
    return getVis(pActor) != nullptr;
}

/**
 * Checks whether the visibility animation ended.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isVisAnimEnd(const LiveActor* pActor) {
    return getVis(pActor)->isAnimEnd();
}

/**
 * Checks whether a visibility animation plays only once.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return Whether the condition holds.
 */
bool isVisAnimOneTime(const LiveActor* pActor, const char* pAnimName) {
    return getVis(pActor)->isAnimOneTime(pAnimName);
}

/**
 * Checks whether the current visibility animation plays only once.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isVisAnimOneTime(const LiveActor* pActor) {
    return getVis(pActor)->isAnimOneTime();
}

/**
 * Gets the playing visibility animation.
 * @param pActor The actor.
 * @return The animation name.
 */
const char* getPlayingVisAnimName(const LiveActor* pActor) {
    return getVis(pActor)->getPlayingAnimName();
}

/**
 * Checks whether a visibility animation is playing.
 * @param pActor The actor.
 * @return Whether the condition holds.
 */
bool isVisAnimPlaying(const LiveActor* pActor) {
    return isVisAnimExist(pActor) && getVis(pActor)->isAnimPlaying();
}

/**
 * Gets the frame of the visibility animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getVisAnimFrame(const LiveActor* pActor) {
    return getVis(pActor)->getAnimFrame();
}

/**
 * Gets the frame rate of the visibility animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getVisAnimFrameRate(const LiveActor* pActor) {
    return getVis(pActor)->getAnimFrameRate();
}

/**
 * Gets the last frame of the visibility animation.
 * @param pActor The actor.
 * @return The value.
 */
f32 getVisAnimFrameMax(const LiveActor* pActor) {
    return getVis(pActor)->getAnimFrameMax();
}

/**
 * Gets the last frame of a visibility animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @return The value.
 */
f32 getVisAnimFrameMax(const LiveActor* pActor, const char* pAnimName) {
    return getVis(pActor)->getAnimFrameMax(pAnimName);
}

/**
 * Stops the visibility animation at a frame.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setVisAnimFrameAndStop(LiveActor* pActor, f32 frame) {
    setVisAnimFrame(pActor, frame);
    setVisAnimFrameRate(pActor, 0.0f);
}

/**
 * Stops the visibility animation at its last frame.
 * @param pActor The actor.
 */
void setVisAnimFrameAndStopEnd(LiveActor* pActor) {
    setVisAnimFrameAndStop(pActor, getVisAnimFrameMax(pActor));
}

/**
 * Creates retargetting info between the skeletons of two actors.
 * @param pActor The actor.
 * @param pTargetActor The actor to retarget to.
 * @param rScale The scale.
 * @return The retargetting info.
 */
SklAnimRetargettingInfo* createSklAnimRetargetting(const LiveActor* pActor, const LiveActor* pTargetActor, const sead::Vector3f& rScale) {
    return new SklAnimRetargettingInfo(pActor->mModelKeeper->mModelCafe->mModelG3D->getModelObj(),
                                       pTargetActor->mModelKeeper->mModelCafe->mModelG3D->getModelObj(),
                                       rScale);
}

/**
 * Creates retargetting info between an actor skeleton and the skeleton of an archive.
 * @param pActor The actor.
 * @param pArchiveName The archive name.
 * @param rScale The scale.
 * @return The retargetting info.
 */
SklAnimRetargettingInfo* createSklAnimRetargetting(const LiveActor* pActor, const char* pArchiveName, const sead::Vector3f& rScale) {
    const nn::g3d::ResSkeleton* skeleton = pActor->mModelKeeper->mModelCafe->getResModel()->GetSkeleton();
    const nn::g3d::ResSkeleton* targetSkeleton =
        findOrCreateResource(pArchiveName, nullptr)->getResFile()->GetModel(0)->GetSkeleton();
    return new SklAnimRetargettingInfo(skeleton, targetSkeleton, rScale);
}

/**
 * Binds retargetting info to the skeletal animation player.
 * @param pActor The actor.
 * @param pInfo The retargetting info.
 */
void bindSklAnimRetargetting(const LiveActor* pActor, const SklAnimRetargettingInfo* pInfo) {
    getSkl(pActor)->mRetargettingInfo = pInfo;
}

/**
 * Unbinds the retargetting info of the skeletal animation player.
 * @param pActor The actor.
 */
void unbindSklAnimRetargetting(const LiveActor* pActor) {
    getSkl(pActor)->mRetargettingInfo = nullptr;
}

/**
 * Initializes partial skeletal animation.
 * @param pActor The actor.
 * @param slotNum The number of slots.
 * @param jointNum The number of joints.
 * @param partsNum The number of parts.
 */
void initPartialSklAnim(LiveActor* pActor, s32 slotNum, s32 jointNum, s32 partsNum) {
    getSkl(pActor)->initPartialAnim(slotNum, jointNum, partsNum);
}

/**
 * Adds a joint range to a partial animation slot.
 * @param pActor The actor.
 * @param pJointName The first joint.
 * @param pEndJointName The last joint.
 * @param partIndex The partial animation slot.
 */
void addPartialSklAnimPartsList(LiveActor* pActor, const char* pJointName, const char* pEndJointName, s32 partIndex) {
    getSkl(pActor)->addPartialAnimJoint(partIndex, pJointName, pEndJointName);
}

/**
 * Adds a joint and its children to a partial animation slot.
 * @param pActor The actor.
 * @param pJointName The joint.
 * @param partIndex The partial animation slot.
 */
void addPartialSklAnimPartsListRecursive(LiveActor* pActor, const char* pJointName, s32 partIndex) {
    getSkl(pActor)->addPartialAnimJointRecursive(partIndex, pJointName);
}

/**
 * Starts a partial skeletal animation.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param partIndex The partial animation slot.
 * @param interpole The interpolation frames.
 * @param pInfo The retargetting info.
 */
void startPartialSklAnim(LiveActor* pActor, const char* pAnimName, s32 partIndex, s32 interpole, const SklAnimRetargettingInfo* pInfo) {
    getSkl(pActor)->startPartialAnim(pAnimName, partIndex, interpole, pInfo);
}

/**
 * Clears a partial skeletal animation.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 */
void clearPartialSklAnim(LiveActor* pActor, s32 partIndex) {
    getSkl(pActor)->clearPartialAnim(partIndex);
}

/**
 * Checks whether a partial skeletal animation ended.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 * @return Whether the condition holds.
 */
bool isPartialSklAnimEnd(LiveActor* pActor, s32 partIndex) {
    return getSkl(pActor)->isPartialAnimEnd(partIndex);
}

/**
 * Checks whether a partial animation slot has an animation.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 * @return Whether the condition holds.
 */
bool isPartialSklAnimAttached(LiveActor* pActor, s32 partIndex) {
    return getSkl(pActor)->isPartialAnimAttached(partIndex);
}

/**
 * Gets the frame of a partial skeletal animation.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 * @return The value.
 */
f32 getPartialSklAnimFrame(LiveActor* pActor, s32 partIndex) {
    return getSkl(pActor)->getPartialAnimFrame(partIndex);
}

/**
 * Sets the frame of a partial skeletal animation.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 * @param frame The frame.
 */
void setPartialSklAnimFrame(LiveActor* pActor, s32 partIndex, f32 frame) {
    getSkl(pActor)->setPartialAnimFrame(partIndex, frame);
}

/**
 * Gets the frame rate of a partial skeletal animation.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 * @return The value.
 */
f32 getPartialSklAnimFrameRate(LiveActor* pActor, s32 partIndex) {
    return getSkl(pActor)->getPartialAnimFrameRate(partIndex);
}

/**
 * Sets the frame rate of a partial skeletal animation.
 * @param pActor The actor.
 * @param partIndex The partial animation slot.
 * @param frameRate The frame rate.
 */
void setPartialSklAnimFrameRate(LiveActor* pActor, s32 partIndex, f32 frameRate) {
    getSkl(pActor)->setPartialAnimFrameRate(partIndex, frameRate);
}

/**
 * Sets the base matrix of the model and calculates its animation.
 * @param pActor The actor.
 * @param rMtx The base matrix.
 * @param rScale The scale.
 */
void setBaseMtxAndCalcAnim(LiveActor* pActor, const sead::Matrix34f& rMtx, const sead::Vector3f& rScale) {
    pActor->mModelKeeper->calc(rMtx, rScale);
}
}  // namespace al

namespace alAnimFunction {
/**
 * Gets the frame of an animation of any type.
 * @param pActor The actor.
 * @param type The animation type, or -1 for the first existing one.
 * @return The value.
 */
f32 getAllAnimFrame(const al::LiveActor* pActor, s32 type) {
    switch (type) {
    case -1:
        if (al::isSklAnimExist(pActor)) {
            return al::getSklAnimFrame(pActor, 0);
        }

        if (al::isMclAnimExist(pActor)) {
            return al::getMclAnimFrame(pActor);
        }

        if (al::isMtpAnimExist(pActor)) {
            return al::getMtpAnimFrame(pActor);
        }

        if (al::isMtsAnimExist(pActor)) {
            return al::getMtsAnimFrame(pActor);
        }

        if (al::isVisAnimExist(pActor)) {
            return al::getVisAnimFrame(pActor);
        }

        return 0.0f;
    case 0:
        return al::getSklAnimFrame(pActor, 0);
    case 1:
        return al::getMclAnimFrame(pActor);
    case 2:
        return al::getMtpAnimFrame(pActor);
    case 3:
        return al::getMtsAnimFrame(pActor);
    case 4:
        return al::getVisAnimFrame(pActor);
    default:
        return 0.0f;
    }
}

/**
 * Gets the last frame of an animation of any type.
 * @param pActor The actor.
 * @param pAnimName The animation name.
 * @param type The animation type, or -1 for the first existing one.
 * @return The value.
 */
f32 getAllAnimFrameMax(const al::LiveActor* pActor, const char* pAnimName, s32 type) {
    switch (type) {
    case -1:
        if (al::isSklAnimExist(pActor, pAnimName)) {
            return al::getSklAnimFrameMax(pActor, pAnimName);
        }

        if (al::isMclAnimExist(pActor, pAnimName)) {
            return al::getMclAnimFrameMax(pActor, pAnimName);
        }

        if (al::isMtpAnimExist(pActor, pAnimName)) {
            return al::getMtpAnimFrameMax(pActor, pAnimName);
        }

        if (al::isMtsAnimExist(pActor, pAnimName)) {
            return al::getMtsAnimFrameMax(pActor, pAnimName);
        }

        if (al::isVisAnimExist(pActor, pAnimName)) {
            return al::getVisAnimFrameMax(pActor, pAnimName);
        }

        return 1.0f;
    case 0:
        return al::getSklAnimFrameMax(pActor, pAnimName);
    case 1:
        return al::getMclAnimFrameMax(pActor, pAnimName);
    case 2:
        return al::getMtpAnimFrameMax(pActor, pAnimName);
    case 3:
        return al::getMtsAnimFrameMax(pActor, pAnimName);
    case 4:
        return al::getVisAnimFrameMax(pActor, pAnimName);
    default:
        return 1.0f;
    }
}

/**
 * Gets the frame rate of an animation of any type.
 * @param pActor The actor.
 * @param type The animation type, or -1 for the first existing one.
 * @return The value.
 */
f32 getAllAnimFrameRate(const al::LiveActor* pActor, s32 type) {
    switch (type) {
    case -1:
        if (al::isSklAnimExist(pActor)) {
            return al::getSklAnimFrameRate(pActor, 0);
        }

        if (al::isMclAnimExist(pActor)) {
            return al::getMclAnimFrameRate(pActor);
        }

        if (al::isMtpAnimExist(pActor)) {
            return al::getMtpAnimFrameRate(pActor);
        }

        if (al::isMtsAnimExist(pActor)) {
            return al::getMtsAnimFrameRate(pActor);
        }

        if (al::isVisAnimExist(pActor)) {
            return al::getVisAnimFrameRate(pActor);
        }

        return 1.0f;
    case 0:
        return al::getSklAnimFrameRate(pActor, 0);
    case 1:
        return al::getMclAnimFrameRate(pActor);
    case 2:
        return al::getMtpAnimFrameRate(pActor);
    case 3:
        return al::getMtsAnimFrameRate(pActor);
    case 4:
        return al::getVisAnimFrameRate(pActor);
    default:
        return 1.0f;
    }
}

/**
 * Gets the name of the first playing animation of any type.
 * @param pActor The actor.
 * @return The animation name.
 */
const char* getAllAnimName(const al::LiveActor* pActor) {
    if (al::isSklAnimExist(pActor)) {
        return al::getPlayingSklAnimName(pActor, 0);
    }

    if (al::isMclAnimExist(pActor)) {
        return al::getPlayingMclAnimName(pActor);
    }

    if (al::isMtpAnimExist(pActor)) {
        return al::getPlayingMtpAnimName(pActor);
    }

    if (al::isMtsAnimExist(pActor)) {
        return al::getPlayingMtsAnimName(pActor);
    }

    if (al::isVisAnimExist(pActor)) {
        return al::getPlayingVisAnimName(pActor);
    }

    return nullptr;
}

/**
 * Checks whether a frame was passed during the last step.
 * @param frame The current frame.
 * @param frameRate The frame rate.
 * @param checkFrame The frame to check.
 * @return Whether the condition holds.
 */
bool checkPass(f32 frame, f32 frameRate, f32 checkFrame) {
    return checkFrame >= 0.0f && frame - frameRate < checkFrame && checkFrame <= frame;
}
}  // namespace alAnimFunction
