#include "Project/Se/SeListenerKeeper.hpp"

#include <audio/seadAudio3DListenerNin.h>
#include <gfx/seadProjection.h>

#include "Project/Audio/System/SeadAudio3DMgr.hpp"
#include "Project/Se/SeListener.hpp"
#include "Project/Se/SeListenerParamTargetViewPos.hpp"
#include "Project/Se/SeListenerPoserAdjustMiddlePos.hpp"
#include "Project/Se/SeListenerPoserMiddlePos.hpp"
#include "Project/Se/SeListenerPoserViewPos.hpp"
#include "Project/Se/SeListenerPoserViewPosOffset.hpp"
#include "Project/Se/SeListenerPoserViewPosOffsetFovy.hpp"

#include "Library/Se/Project/SeKeeper.hpp"
#include "Library/Se/Project/SeKeeperInternal.hpp"

#include <audio/seadAudioSoundDataMgrNin.h>

#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Math/InOutParam.hpp"
#include "Library/Se/DataBase/SeDataBase.hpp"
#include "Library/Se/Function/SeDbFunction.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Info/SeSource.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"
#include "Library/Se/Project/ISeModifier.hpp"
#include "Library/Se/Project/SePlayParamList.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Audio/System/AudioResourceLoader.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Se/SeEmitter.hpp"
#include "Project/Se/SeEmitterHolder.hpp"

namespace {
/**
 * @brief Sets the default distance attenuation and biquad parameters.
 * @param rParam Constructed listener parameter block receiving the defaults.
 */
inline void setDefaultListenerParams(sead::Audio3DListenerParameterNin& rParam) {
    rParam.mOutputTypeFlag = 1;
    rParam.mInteriorSize = 1500.0f;
    rParam.mMaxVolumeDistance = 450.0f;
    rParam.mUnitDistance = 1000.0f;
    rParam.mUserParam = 0;
    rParam.mUnitBiquadFilterValue = 0.5f;
    rParam.mMaxBiquadFilterValue = 1.0f;
}
} // namespace

namespace al {
namespace detail {
SeResourceSpecificInfo sDefaultSpecificInfo;
}
using detail::findSpecificInfo;
using detail::sDefaultSpecificInfo;

/** @brief Resolves water and wet-material state. @return The effective material state selector. */
s32 SeKeeper::getWaterState() { return calcWaterState(); }

/**
 * @brief Applies the configured sequence-variable and biquad defaults.
 * @param pParamList Non-null parameter list receiving the defaults.
 * @param id Unused in this version.
 * @param isHold Unused in this version.
 */
void SeKeeper::applyKeeperParamsToParamList(SePlayParamList* pParamList, u32 id, bool isHold) {
    for (s32 i = 0; i < 4; i++) {
        SeqLocalVariableDefault* pVariable = mSeqLocalVariables[i];

        if (pVariable->index >= 0) {
            pParamList->setLocalVariable(pVariable->value, pVariable->index);
        }
    }

    if (mBiquadFilter->type != 0) {
        pParamList->setBiquadFilter(mBiquadFilter->value, mBiquadFilter->type);
    }
}

/**
 * @brief Refreshes a held sound request and applies keeper defaults.
 * @param id Sound identifier to request or stop.
 * @param pEmitterName Optional emitter name; nullptr selects the first emitter.
 * @param pMeInfo Optional music-effect information forwarded to the sound director.
 * @param pSpecificInfo Optional resource-specific settings; nullptr selects database settings or shared
 * defaults.
 * @param pPlayName Optional request-keeper name; nullptr selects the keeper default.
 * @return Resulting parameter list, or nullptr when no request succeeds.
 */
SePlayParamList* SeKeeper::requestHoldSe(u32 id, const char* pEmitterName, MeInfo* pMeInfo,
                                         const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName) {
    SeEmitter* pEmitter = mEmitterHolder->findEmitter(pEmitterName);
    u32 soundId = id;

    if (mModifier != nullptr) {
        soundId = mModifier->modifySoundId(id);

        if (soundId == AudioConst::SOUND_ID_INVALID) {
            return nullptr;
        }
    }

    if (pSpecificInfo == nullptr) {
        pSpecificInfo = findSpecificInfo(mSeDataBase, id);
    }

    if (pPlayName == nullptr) {
        pPlayName = mPlayName;
    }

    SePlayParamList* pParamList =
        mSeDirector->addHoldRequest(soundId, pEmitter->getSeSource(), pSpecificInfo, pMeInfo, mMaterialName,
                                    getWaterState(), mIsBeyondWall, pPlayName);
    mEmitterHolder->setIsActive(true);
    pEmitter->activate();

    if (pParamList == nullptr) {
        return pParamList;
    }

    if (alSoundNameUtil::getSoundType(soundId, false) == 1) {
        applyKeeperParamsToParamList(pParamList, soundId, true);

        if (mModifier != nullptr) {
            mModifier->modifyHoldParam(soundId, pParamList);
        }
    }

    return pParamList;
}

/**
 * @brief Requests playback for a sound identifier or named play definition.
 * @param pName Optional null-terminated play-definition name.
 * @param pMeInfo Optional music-effect information forwarded to the sound director.
 * @param isHold Whether to issue a held sound request.
 * @return Number of resources for which playback was requested.
 */
s32 SeKeeper::requestPlaySe(const char* pName, MeInfo* pMeInfo, bool isHold) {
    return requestPlaySeFromName(pName, pMeInfo, isHold);
}

/**
 * @brief Requests every resource in a named play definition.
 * @param pName Optional null-terminated play-definition name.
 * @param pMeInfo Optional music-effect information forwarded to the sound director.
 * @param isHold Whether to issue a held sound request.
 * @return Number of resources for which playback was requested.
 */
s32 SeKeeper::requestPlaySeFromName(const char* pName, MeInfo* pMeInfo, bool isHold) {
    s32 playNum = 0;

    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return playNum;
    }

    const SePlayInfo* pPlayInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (pPlayInfo == nullptr || pPlayInfo->mResourceInfoList == nullptr) {
        return playNum;
    }

    s32 resourceNum = pPlayInfo->mResourceInfoList->getInfoNum();

    if (resourceNum < 1) {
        return playNum;
    }

    if (isHold) {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList != nullptr
                                                      ? pPlayInfo->mResourceInfoList->tryGetInfo(i)
                                                      : nullptr;
            if (pResourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* pSpecificInfo = pResourceInfo->mSpecificInfo != nullptr
                                                              ? pResourceInfo->mSpecificInfo
                                                              : &sDefaultSpecificInfo;
            SePlayParamList* pParamList =
                requestHoldSe(pResourceInfo->mSoundId, pResourceInfo->mEmitterName, pMeInfo, pSpecificInfo,
                              pPlayInfo->mRequestKeeperName);
            if (pResourceInfo->mLfeSend > 0.0f) {
                pParamList->setSpeakerVolumeLfeCenter(pResourceInfo->mLfeSend, 0.0f);
            }

            playNum++;
        }
    } else {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList != nullptr
                                                      ? pPlayInfo->mResourceInfoList->tryGetInfo(i)
                                                      : nullptr;
            if (pResourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* pSpecificInfo = pResourceInfo->mSpecificInfo != nullptr
                                                              ? pResourceInfo->mSpecificInfo
                                                              : &sDefaultSpecificInfo;
            SePlayParamList* pParamList =
                requestPlaySe(pResourceInfo->mSoundId, pResourceInfo->mEmitterName, pPlayInfo->mIsLoop,
                              pMeInfo, pSpecificInfo, pPlayInfo->mRequestKeeperName);
            if (pResourceInfo->mLfeSend > 0.0f) {
                pParamList->setSpeakerVolumeLfeCenter(pResourceInfo->mLfeSend, 0.0f);
            }

            playNum++;
        }
    }

    return playNum;
}

/**
 * @brief Requests named resources with volume, pitch, and tempo driven by an input value.
 * @param pName Optional null-terminated play-definition name.
 * @param param Input value used by the resource parameter mappings.
 * @param pMeInfo Optional music-effect information forwarded to the sound director.
 * @param isHold Whether to issue a held sound request.
 * @param isTry Whether a request with no counted playback may still return its first parameter list.
 * @return Resulting parameter list, or nullptr when no request succeeds.
 */
SePlayParamList* SeKeeper::requestPlaySeFromNameWithParam(const char* pName, f32 param, MeInfo* pMeInfo,
                                                          bool isHold, bool isTry) {
    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return nullptr;
    }

    const SePlayInfo* pPlayInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (pPlayInfo == nullptr) {
        return nullptr;
    }

    SePlayParamList* pFirstParamList = nullptr;
    s32 playNum = 0;
    s32 resourceNum =
        pPlayInfo->mResourceInfoList != nullptr ? pPlayInfo->mResourceInfoList->getInfoNum() : 0;

    for (s32 i = 0; i < resourceNum; i++) {
        const SeResourceInfo* pResourceInfo =
            pPlayInfo->mResourceInfoList != nullptr ? pPlayInfo->mResourceInfoList->tryGetInfo(i) : nullptr;
        if (pResourceInfo == nullptr) {
            continue;
        }

        u32 soundId = pResourceInfo->mSoundId;
        const SeResourceSpecificInfo* pSpecificInfo =
            pResourceInfo->mSpecificInfo != nullptr ? pResourceInfo->mSpecificInfo : &sDefaultSpecificInfo;
        f32 input = alSeDbFunction::convertSeInputParam(
            static_cast<SeInputFunctionId>(pResourceInfo->mInputFunctionId), param);
        if (pResourceInfo->mVolume != nullptr && input < pResourceInfo->mVolume->getInMin()) {
            playNum++;
            continue;
        }

        SePlayParamList* pParamList;

        if (isHold) {
            pParamList = requestHoldSe(soundId, pResourceInfo->mEmitterName, pMeInfo, pSpecificInfo,
                                       pPlayInfo->mRequestKeeperName);
        } else {
            pParamList = requestPlaySe(soundId, pResourceInfo->mEmitterName, pPlayInfo->mIsLoop, pMeInfo,
                                       pSpecificInfo, pPlayInfo->mRequestKeeperName);
        }

        if (pParamList == nullptr) {
            continue;
        }

        if (pFirstParamList == nullptr) {
            pFirstParamList = pParamList;
        }

        if (pResourceInfo->mIsSetParamMin && pResourceInfo->mParamMin > input) {
            playNum++;
            continue;
        }

        if (pResourceInfo->mVolume != nullptr) {
            pParamList->setVolume(alSeDbFunction::calcLeapValue(pResourceInfo->mVolume, input));
        }

        if (pResourceInfo->mPitch != nullptr) {
            pParamList->setPitch(alSeDbFunction::calcLeapValue(pResourceInfo->mPitch, input));
        }

        if (pResourceInfo->mTempo != nullptr) {
            pParamList->setTempo(alSeDbFunction::calcLeapValue(pResourceInfo->mTempo, input));
        }

        if (pResourceInfo->mLocalVarNo >= 0) {
            pParamList->setLocalVariable(static_cast<s32>(input), pResourceInfo->mLocalVarNo);
        }

        if (pResourceInfo->mLfeSend > 0.0f) {
            pParamList->setSpeakerVolumeLfeCenter(pResourceInfo->mLfeSend, 0.0f);
        }

        playNum++;
    }

    if (playNum < 1 && !isTry) {
        return nullptr;
    }

    return pFirstParamList;
}

/**
 * @brief Requests named resources and returns the first resulting parameter list.
 * @param pName Optional null-terminated play-definition name.
 * @param pMeInfo Optional music-effect information forwarded to the sound director.
 * @param isHold Whether to issue a held sound request.
 * @return Resulting parameter list, or nullptr when no request succeeds.
 */
SePlayParamList* SeKeeper::requestPlaySeFromNameGetParamList(const char* pName, MeInfo* pMeInfo,
                                                             bool isHold) {
    SePlayParamList* pFirstParamList = nullptr;

    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return pFirstParamList;
    }

    const SePlayInfo* pPlayInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (pPlayInfo == nullptr || pPlayInfo->mResourceInfoList == nullptr) {
        return pFirstParamList;
    }

    s32 resourceNum = pPlayInfo->mResourceInfoList->getInfoNum();

    if (resourceNum < 1) {
        return pFirstParamList;
    }

    if (isHold) {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList != nullptr
                                                      ? pPlayInfo->mResourceInfoList->tryGetInfo(i)
                                                      : nullptr;
            if (pResourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* pSpecificInfo = pResourceInfo->mSpecificInfo != nullptr
                                                              ? pResourceInfo->mSpecificInfo
                                                              : &sDefaultSpecificInfo;
            SePlayParamList* pParamList =
                requestHoldSe(pResourceInfo->mSoundId, pResourceInfo->mEmitterName, pMeInfo, pSpecificInfo,
                              pPlayInfo->mRequestKeeperName);
            if (pParamList != nullptr && pFirstParamList == nullptr) {
                pFirstParamList = pParamList;
            }
        }
    } else {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList != nullptr
                                                      ? pPlayInfo->mResourceInfoList->tryGetInfo(i)
                                                      : nullptr;
            if (pResourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* pSpecificInfo = pResourceInfo->mSpecificInfo != nullptr
                                                              ? pResourceInfo->mSpecificInfo
                                                              : &sDefaultSpecificInfo;
            SePlayParamList* pParamList =
                requestPlaySe(pResourceInfo->mSoundId, pResourceInfo->mEmitterName, pPlayInfo->mIsLoop,
                              pMeInfo, pSpecificInfo, pPlayInfo->mRequestKeeperName);
            if (pParamList != nullptr && pFirstParamList == nullptr) {
                pFirstParamList = pParamList;
            }
        }
    }

    return pFirstParamList;
}

/**
 * @brief Stops the requested sound or named play definition.
 * @param id Sound identifier to request or stop.
 * @param fadeFrames Fade duration in frames; nonpositive values may select the play definition default.
 * @param pEmitterName Optional emitter name; nullptr selects the first emitter.
 * @param pPlayName Optional request-keeper name; nullptr selects the keeper default.
 */
void SeKeeper::stopSe(u32 id, s32 fadeFrames, const char* pEmitterName, const char* pPlayName) {
    if (mUserInfo == nullptr) {
        return;
    }

    if (pPlayName == nullptr) {
        pPlayName = mPlayName;
    }

    mSeDirector->stop(id, mEmitterHolder->findEmitter(pEmitterName)->getSeSource(), fadeFrames, pPlayName);
}

/**
 * @brief Stops the resources of a named play definition using the selected stop operation.
 * @param pName Optional null-terminated play-definition name.
 * @param fadeFrames Fade duration in frames; nonpositive values may select the play definition default.
 * @param pPlayName Optional request-keeper name; nullptr selects the keeper default.
 * @param isSingle True selects stop; false selects stopAllId for each resource.
 */
void SeKeeper::stopAllSeFromName(const char* pName, s32 fadeFrames, const char* pPlayName, bool isSingle) {
    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return;
    }

    const SePlayInfo* pPlayInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (pPlayInfo == nullptr) {
        return;
    }

    if (fadeFrames <= 0) {
        s32 defaultFrames = pPlayInfo->mFadeOutFrameNum;
        fadeFrames = SePlayInfo::USE_DEFAULT_FADE_OUT_FRAME_NUM == defaultFrames ? 0 : defaultFrames;
    }

    if (pPlayInfo->mResourceInfoList == nullptr) {
        return;
    }

    s32 resourceNum = pPlayInfo->mResourceInfoList->getInfoNum();

    if (resourceNum < 1) {
        return;
    }

    if (isSingle) {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList->getInfo(i);
            const char* pEmitterName = pResourceInfo->mEmitterName;
            u32 soundId = pResourceInfo->mSoundId;

            if (mEmitterHolder->findEmitter(pEmitterName) != nullptr) {
                mSeDirector->stop(soundId, mEmitterHolder->findEmitter(pEmitterName)->getSeSource(),
                                  fadeFrames, pPlayName);
            }
        }
    } else {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList->findInfoDirect(i);
            const char* pEmitterName = pResourceInfo->mEmitterName;
            u32 soundId = pResourceInfo->mSoundId;

            if (mEmitterHolder->findEmitter(pEmitterName) != nullptr) {
                mSeDirector->stopAllId(soundId, mEmitterHolder->findEmitter(pEmitterName)->getSeSource(),
                                       fadeFrames, pPlayName);
            }
        }
    }
}

/**
 * @brief Stops the requested sound or named play definition.
 * @param pName Optional null-terminated play-definition name.
 */
void SeKeeper::stopSe(const char* pName) { stopSeFromName(pName); }

/**
 * @brief Stops all resources in the named play definition with its configured fade.
 * @param pName Optional null-terminated play-definition name.
 */
void SeKeeper::stopSeFromName(const char* pName) {
    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return;
    }

    const SePlayInfo* pPlayInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (pPlayInfo == nullptr) {
        return;
    }

    s32 fadeOutFrames = pPlayInfo->mFadeOutFrameNum;
    s32 fadeFrames = SePlayInfo::USE_DEFAULT_FADE_OUT_FRAME_NUM == fadeOutFrames ? 0 : fadeOutFrames;

    if (pPlayInfo->mResourceInfoList == nullptr) {
        return;
    }

    s32 resourceNum = pPlayInfo->mResourceInfoList->getInfoNum();

    for (s32 i = 0; i < resourceNum; i++) {
        const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList->findInfoDirect(i);
        const char* pEmitterName = pResourceInfo->mEmitterName;
        u32 soundId = pResourceInfo->mSoundId;

        if (mEmitterHolder->findEmitter(pEmitterName) != nullptr) {
            const char* pPlayName = pPlayInfo->mRequestKeeperName;

            if (pPlayName == nullptr) {
                pPlayName = mPlayName;
            }

            mSeDirector->stop(soundId, mEmitterHolder->findEmitter(pEmitterName)->getSeSource(), fadeFrames,
                              pPlayName);
        }
    }
}

/**
 * @brief Stops sounds from every emitter owned by the keeper.
 * @param fadeFrames Fade duration in frames; nonpositive values may select the play definition default.
 */
void SeKeeper::stopAll(s32 fadeFrames) {
    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->stopAllFromSource(alSeFunction::getSeSource(mEmitterHolder, i), fadeFrames);
    }
}

/**
 * @brief Enables the keeper and reactivates its emitter sources.
 */
void SeKeeper::activate() {
    mIsActive = true;

    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->reactivateSeFromSource(mEmitterHolder->getEmitter(i)->getSeSource());
    }
}

/**
 * @brief Disables the keeper and stops or pauses its emitter sources.
 * @param isClipped Whether clipping permits sounds configured to pause at a distance to be paused.
 */
void SeKeeper::deactivate(bool isClipped) {
    mIsActive = false;

    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->deactivateSeFromSource(alSeFunction::getSeSource(mEmitterHolder, i), 0, isClipped);
    }
}

/**
 * @brief Resets emitter velocity when an emitter holder exists.
 */
void SeKeeper::resetVelocity() {
    if (mUserInfo != nullptr) {
        mEmitterHolder->resetVelocity();
    }
}

/**
 * @brief Updates the water state and notifies all emitter sources when it changes.
 * @param isInWater New underwater-state flag.
 */
void SeKeeper::setIsInWater(bool isInWater) {
    if (mIsInWater == isInWater) {
        return;
    }

    mIsInWater = isInWater;

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->notifiedUpdateMaterial(alSeFunction::getSeSource(mEmitterHolder, i), mMaterialName,
                                            getWaterState());
    }
}

/**
 * @brief Updates the wet-material state and notifies emitter sources when it changes.
 * @param isWet New wet-material flag.
 */
void SeKeeper::setIsMaterialWet(bool isWet) {
    if (mIsMaterialWet == isWet) {
        return;
    }

    mIsMaterialWet = isWet;

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->notifiedUpdateMaterial(alSeFunction::getSeSource(mEmitterHolder, i), mMaterialName,
                                            getWaterState());
    }
}

/**
 * @brief Updates the single-mode wet state and notifies emitter sources when it changes.
 * @param isWet New wet-material flag.
 */
void SeKeeper::setIsMaterialWetSingleMode(bool isWet) {
    if (mIsMaterialWetSingleMode == isWet) {
        return;
    }

    mIsMaterialWetSingleMode = isWet;

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->notifiedUpdateMaterial(alSeFunction::getSeSource(mEmitterHolder, i), mMaterialName,
                                            getWaterState());
    }
}

/**
 * @brief Changes the material name and notifies sources when the material differs.
 * @param pMaterialName Optional null-terminated material name; nullptr clears the material.
 */
void SeKeeper::tryUpdateMaterial(const char* pMaterialName) {
    if (pMaterialName != nullptr) {
        if (mMaterialName != nullptr && isEqualString(pMaterialName, mMaterialName)) {
            return;
        }
    } else if (mMaterialName == nullptr) {
        return;
    }

    mMaterialName = pMaterialName;

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->notifiedUpdateMaterial(alSeFunction::getSeSource(mEmitterHolder, i), mMaterialName,
                                            getWaterState());
    }
}

/**
 * @brief Updates an existing sequence-variable default or uses the first free slot.
 * @param index Sequence variable index; four default slots are available.
 * @param value Value to store for the selected default.
 */
void SeKeeper::setSeqLocalVariableDefault(s32 index, s32 value) {
    for (s32 i = 0; i < 4; ++i) {
        SeqLocalVariableDefault* pVariable = mSeqLocalVariables[i];
        if (pVariable->index == index) {
            pVariable->value = value;
            mSeqLocalVariables[i]->index = index;
            return;
        }
    }
    for (s32 i = 0; i < 4; ++i) {
        SeqLocalVariableDefault* pVariable = mSeqLocalVariables[i];
        if (pVariable->index < 0) {
            pVariable->value = value;
            mSeqLocalVariables[i]->index = index;
            return;
        }
    }
}

/**
 * @brief Stores the biquad filter type and value for subsequent requests.
 * @param type Biquad filter type; zero disables the default override.
 * @param value Value to store for the selected default.
 */
void SeKeeper::setBiquadFilterDefault(s32 type, f32 value) {
    mBiquadFilter->type = type;
    mBiquadFilter->value = value;
}

/**
 * @brief Applies a volume value to each emitter source.
 * @param volume Source volume value; forwarded without clamping.
 */
void SeKeeper::setSeSourceVolume(f32 volume) {
    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        alSeFunction::getSeSource(mEmitterHolder, i)->setVolume(volume);
    }
}

/**
 * @brief Requests loading of every sound resource used by this keeper.
 * @param pLoader Non-null resource loader receiving sound load requests.
 */
void SeKeeper::loadSe(IAudioResourceLoader* pLoader) {
    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < (mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->getInfoNum() : 0);
         i++) {
        const SePlayInfo* pPlayInfo =
            mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->tryGetInfo(i) : nullptr;
        if (pPlayInfo == nullptr) {
            continue;
        }

        for (s32 j = 0;
             j < (pPlayInfo->mResourceInfoList != nullptr ? pPlayInfo->mResourceInfoList->getInfoNum() : 0);
             j++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList != nullptr
                                                      ? pPlayInfo->mResourceInfoList->tryGetInfo(j)
                                                      : nullptr;
            if (pResourceInfo == nullptr) {
                continue;
            }

            pLoader->loadSoundItem(pResourceInfo->mSoundId, -1);
        }
    }
}

/**
 * @brief Queries whether every referenced sound resource is loaded.
 * @param pPlayer Non-null audio player supplying the sound-data manager.
 * @param pName Unused diagnostic play name.
 */
void SeKeeper::verifySe(SeadAudioPlayer* pPlayer, const char* pName) {
    if (mUserInfo == nullptr) {
        return;
    }

    auto* pDataMgr = pPlayer->getSoundDataMgr();

    for (s32 i = 0; i < (mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->getInfoNum() : 0);
         i++) {
        const SePlayInfo* pPlayInfo =
            mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->tryGetInfo(i) : nullptr;
        if (pPlayInfo == nullptr) {
            continue;
        }

        for (s32 j = 0;
             j < (pPlayInfo->mResourceInfoList != nullptr ? pPlayInfo->mResourceInfoList->getInfoNum() : 0);
             j++) {
            const SeResourceInfo* pResourceInfo = pPlayInfo->mResourceInfoList != nullptr
                                                      ? pPlayInfo->mResourceInfoList->tryGetInfo(j)
                                                      : nullptr;
            if (pResourceInfo == nullptr) {
                continue;
            }

            pDataMgr->IsDataLoaded(pResourceInfo->mSoundId, -1);
        }
    }
}
} // namespace al

namespace al {
/**
 * @brief Creates the listener and its posers.
 * @param isUnused Unused.
 * @param listenerNum Number of listeners.
 */
SeListenerKeeper::SeListenerKeeper(bool isUnused, s32 listenerNum) {
    mListeners.allocBuffer(listenerNum, nullptr);
    SeListener* pListener = new SeListener(10);
    mListeners.pushBack(pListener);
    pListener->addPoser(new SeListenerPoserViewPos("カメラ位置", "CameraPosition"));
    pListener->addPoser(
        new SeListenerPoserMiddlePos("ターゲット寄り中間", "MiddlePositionCloseToTarget", 0.8f));
    pListener->addPoser(new SeListenerPoserMiddlePos("コースセレクト用", "CourseSelect", 0.94f));
    pListener->addPoser(new SeListenerPoserMiddlePos("キノピオ探検隊用", "ForKinopioBrigadeMembers", 0.5f));
    pListener->addPoser(new SeListenerPoserMiddlePos("固定カメラ小部屋用", "FixedCameraForSmallRooms", 0.5f));
    pListener->addPoser(new SeListenerPoserMiddlePos("回転部屋用", "ForRotatingRooms", 0.3f));
    pListener->addPoser(new SeListenerPoserMiddlePos("カメラ寄り中間", "MiddlePositionCloseToCamera", 0.2f));
    pListener->addPoser(new SeListenerPoserAdjustMiddlePos("可変中間位置", "AdjustableMiddlePosition"));
    pListener->addPoser(new SeListenerPoserViewPosOffset("カメラ位置オフセット", "CameraPositionOffset",
                                                         sead::Vector3f(0.0f, 0.0f, 0.0f)));
    pListener->addPoser(
        new SeListenerPoserViewPosOffsetFovy("カメラ位置オフセットFovy", "CameraPositionOffsetFovy"));
}

/**
 * @brief Sets the camera the listener follows and the default listener parameters.
 * @param pMgr 3D audio manager.
 * @param pCameraPos Camera position.
 * @param pCameraMtx Camera view matrix.
 * @param pProjection Camera projection.
 * @param pCameraAt Camera target position.
 * @param pPoserName Default poser name, or nullptr.
 */
void SeListenerKeeper::init(SeadAudio3DMgr* pMgr, const sead::Vector3f* pCameraPos,
                            const sead::Matrix34f* pCameraMtx, sead::PerspectiveProjection* pProjection,
                            const sead::Vector3f* pCameraAt, const char* pPoserName) {
    mListenerParam = new SeListenerParamTargetViewPos(pCameraPos, pCameraMtx, pProjection, pCameraAt);
    mAudio3DMgr = pMgr;
    mDefaultPoserName = pPoserName != nullptr ? pPoserName : "ターゲット寄り中間";
    mListeners.unsafeAt(0)->setCurrentPoser(mDefaultPoserName);
    sead::Audio3DListenerParameterNin param;
    setDefaultListenerParams(param);
    mAudio3DMgr->setDefaultListenerParameter(param);
}

/**
 * @brief Updates the listener matrix.
 */
void SeListenerKeeper::update() {
    SeListener* pListener = mListeners.unsafeAt(0);
    pListener->calcListenerMatrix(*mListenerParam);
    mAudio3DMgr->setDefaultListenerMatrix(pListener->getListenerMatrix());
}

/**
 * @brief Changes the listener parameters, replacing negative values by the defaults.
 * @param rParam Listener parameters.
 */
void SeListenerKeeper::changeListenerParam(sead::Audio3DListenerParameterNin& rParam) {
    if (rParam.mInteriorSize < 0.0f) {
        rParam.mInteriorSize = 1500.0f;
    }

    if (rParam.mMaxVolumeDistance < 0.0f) {
        rParam.mMaxVolumeDistance = 450.0f;
    }

    if (rParam.mUnitDistance < 0.0f) {
        rParam.mUnitDistance = 1000.0f;
    }

    if (rParam.mUnitBiquadFilterValue < 0.0f) {
        rParam.mUnitBiquadFilterValue = 0.5f;
    }

    if (rParam.mMaxBiquadFilterValue < 0.0f) {
        rParam.mMaxBiquadFilterValue = 1.0f;
    }

    mAudio3DMgr->setDefaultListenerParameter(rParam);
}

/**
 * @brief Restores the default listener parameters.
 */
void SeListenerKeeper::resetListenerParam() {
    sead::Audio3DListenerParameterNin param;
    setDefaultListenerParams(param);
    mAudio3DMgr->setDefaultListenerParameter(param);
}

/**
 * @brief Changes the listener poser and remembers the current one.
 * @param pName Poser name, or nullptr for the default poser.
 */
void SeListenerKeeper::changeListenerPoser(const char* pName) {
    mLastPoserName = mListeners.unsafeAt(0)->getCurrentPoser()->getName().cstr();

    if (pName != nullptr) {
        mListeners.unsafeAt(0)->setCurrentPoser(pName);
    } else {
        mListeners.unsafeAt(0)->setCurrentPoser(mDefaultPoserName);
    }
}

/**
 * @brief Changes the listener poser back to the remembered one.
 */
void SeListenerKeeper::changeListenerPoserToLast() {
    const char* lastName = mLastPoserName;
    mLastPoserName = mListeners.unsafeAt(0)->getCurrentPoser()->getName().cstr();

    if (lastName != nullptr) {
        mListeners.unsafeAt(0)->setCurrentPoser(lastName);
    } else {
        mListeners.unsafeAt(0)->setCurrentPoser(mDefaultPoserName);
    }
}
} // namespace al
