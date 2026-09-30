#include "Library/Se/Project/SeKeeper.hpp"

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
al::SeResourceSpecificInfo sDefaultSpecificInfo;

const al::SeResourceSpecificInfo* findSpecificInfo(const al::SeDataBase* pDataBase, u32 id) {
    const al::SeResourceSpecificInfo* info = nullptr;
    const al::AudioInfoList<al::SeResourceSpecificInfo>* list = pDataBase->getResourceSpecificInfoList();

    if (list != nullptr && al::AudioConst::SOUND_ID_INVALID != id) {
        do {
            for (s32 i = 0; i < list->mInfos->size(); i++) {
                if (list->mInfos->unsafeAt(i)->mSoundId == id) {
                    info = list->mInfos->unsafeAt(i);
                    goto found;
                }
            }

            list = list->mNext;
        } while (list != nullptr);
    }

found:
    return info != nullptr ? info : &sDefaultSpecificInfo;
}
}  // namespace

namespace al {
SeKeeper::SeKeeper(AudioSystemInfo* pInfo, SeDirector* pDirector, const char* pUserName,
                   const sead::Vector3f* pTrans, const sead::Matrix34f* pMtx, const ModelKeeper* pModelKeeper,
                   const char* pPlayName)
    : mSeDirector(pDirector), mUserName(pUserName), mModelKeeper(pModelKeeper), mPlayName(pPlayName) {
    mSeqLocalVariables = new SeqLocalVariableDefault*[4];

    for (s32 i = 0; i < 4; i++) {
        mSeqLocalVariables[i] = new SeqLocalVariableDefault;
    }

    mBiquadFilter = new BiquadFilterDefault;

    bool isUseModel;

    if (pMtx != nullptr) {
        mPose = new SeSourcePose3DMtxPtr(pMtx);
        isUseModel = true;
    } else if (pTrans != nullptr) {
        mPose = new SeSourcePose3DPosPtr(pTrans);
        isUseModel = true;
    } else {
        mPose = new SeSourcePoseNull();
        isUseModel = false;
    }

    mSeDataBase = pInfo->mSeDataBase;

    if (pInfo->mSeDataBase->getUserInfoList() != nullptr && mUserName != nullptr) {
        mUserInfo = pInfo->mSeDataBase->getUserInfoList()->tryFindInfo(mUserName);

        if (mUserInfo != nullptr) {
            mEmitterHolder = new SeEmitterHolder(pInfo, mUserName, mUserInfo->mEmitterInfoList, mModelKeeper,
                                                 mPose, isUseModel);
        }
    } else {
        mUserInfo = nullptr;
    }
}

void SeKeeper::update() {
    if (mIsActive && mUserInfo != nullptr && mEmitterHolder->isActive()) {
        mEmitterHolder->update();
    }
}

SePlayParamList* SeKeeper::requestPlaySe(u32 id, const char* pEmitterName, bool isHold, MeInfo* pMeInfo,
                                         const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName) {
    SeEmitter* emitter = mEmitterHolder->findEmitter(pEmitterName);
    mEmitterHolder->setIsActive(true);
    emitter->activate();
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

    SePlayParamList* paramList = mSeDirector->addRequest(soundId, emitter->getSeSource(), isHold, pSpecificInfo,
                                                         pMeInfo, mMaterialName, getWaterState(), mIsBeyondWall,
                                                         pPlayName);
    if (paramList == nullptr) {
        return paramList;
    }

    if (alSoundNameUtil::getSoundType(soundId, false) == 1) {
        applyKeeperParamsToParamList(paramList, soundId, false);

        if (mModifier != nullptr) {
            mModifier->modifyStartParam(soundId, paramList);
        }
    }

    return paramList;
}

s32 SeKeeper::getWaterState() {
    u8 state = mIsMaterialWet;

    if (mIsMaterialWetSingleMode) {
        state = 3;
    }

    if (mIsInWater) {
        state = 2;
    }

    return state;
}

void SeKeeper::applyKeeperParamsToParamList(SePlayParamList* pParamList, u32 id, bool isHold) {
    for (s32 i = 0; i < 4; i++) {
        SeqLocalVariableDefault* variable = mSeqLocalVariables[i];

        if (variable->index >= 0) {
            pParamList->setLocalVariable(variable->value, variable->index);
        }
    }

    if (mBiquadFilter->type != 0) {
        pParamList->setBiquadFilter(mBiquadFilter->value, mBiquadFilter->type);
    }
}

SePlayParamList* SeKeeper::requestHoldSe(u32 id, const char* pEmitterName, MeInfo* pMeInfo,
                                         const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName) {
    SeEmitter* emitter = mEmitterHolder->findEmitter(pEmitterName);
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

    SePlayParamList* paramList = mSeDirector->addHoldRequest(soundId, emitter->getSeSource(), pSpecificInfo,
                                                             pMeInfo, mMaterialName, getWaterState(), mIsBeyondWall,
                                                             pPlayName);
    mEmitterHolder->setIsActive(true);
    emitter->activate();

    if (paramList == nullptr) {
        return paramList;
    }

    if (alSoundNameUtil::getSoundType(soundId, false) == 1) {
        applyKeeperParamsToParamList(paramList, soundId, true);

        if (mModifier != nullptr) {
            mModifier->modifyHoldParam(soundId, paramList);
        }
    }

    return paramList;
}

s32 SeKeeper::requestPlaySe(const char* pName, MeInfo* pMeInfo, bool isHold) {
    return requestPlaySeFromName(pName, pMeInfo, isHold);
}

s32 SeKeeper::requestPlaySeFromName(const char* pName, MeInfo* pMeInfo, bool isHold) {
    s32 playNum = 0;

    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return playNum;
    }

    const SePlayInfo* playInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (playInfo == nullptr || playInfo->mResourceInfoList == nullptr) {
        return playNum;
    }

    s32 resourceNum = playInfo->mResourceInfoList->getInfoNum();

    if (resourceNum < 1) {
        return playNum;
    }

    if (isHold) {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* resourceInfo =
                playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(i) : nullptr;
            if (resourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* specificInfo =
                resourceInfo->mSpecificInfo != nullptr ? resourceInfo->mSpecificInfo : &sDefaultSpecificInfo;
            SePlayParamList* paramList = requestHoldSe(resourceInfo->mSoundId, resourceInfo->mEmitterName, pMeInfo,
                                                       specificInfo, playInfo->mRequestKeeperName);
            if (resourceInfo->mLfeSend > 0.0f) {
                paramList->setSpeakerVolumeLfeCenter(resourceInfo->mLfeSend, 0.0f);
            }

            playNum++;
        }
    } else {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* resourceInfo =
                playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(i) : nullptr;
            if (resourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* specificInfo =
                resourceInfo->mSpecificInfo != nullptr ? resourceInfo->mSpecificInfo : &sDefaultSpecificInfo;
            SePlayParamList* paramList =
                requestPlaySe(resourceInfo->mSoundId, resourceInfo->mEmitterName, playInfo->mIsLoop, pMeInfo,
                              specificInfo, playInfo->mRequestKeeperName);
            if (resourceInfo->mLfeSend > 0.0f) {
                paramList->setSpeakerVolumeLfeCenter(resourceInfo->mLfeSend, 0.0f);
            }

            playNum++;
        }
    }

    return playNum;
}

SePlayParamList* SeKeeper::requestPlaySeFromNameWithParam(const char* pName, f32 param, MeInfo* pMeInfo, bool isHold,
                                                          bool isTry) {
    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return nullptr;
    }

    const SePlayInfo* playInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (playInfo == nullptr) {
        return nullptr;
    }

    SePlayParamList* firstParamList = nullptr;
    s32 playNum = 0;
    s32 resourceNum = playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->getInfoNum() : 0;

    for (s32 i = 0; i < resourceNum; i++) {
        const SeResourceInfo* resourceInfo =
            playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(i) : nullptr;
        if (resourceInfo == nullptr) {
            continue;
        }

        u32 soundId = resourceInfo->mSoundId;
        const SeResourceSpecificInfo* specificInfo =
            resourceInfo->mSpecificInfo != nullptr ? resourceInfo->mSpecificInfo : &sDefaultSpecificInfo;
        f32 input = alSeDbFunction::convertSeInputParam(
            static_cast<SeInputFunctionId>(resourceInfo->mInputFunctionId), param);
        if (resourceInfo->mVolume != nullptr && input < resourceInfo->mVolume->getInMin()) {
            playNum++;
            continue;
        }

        SePlayParamList* paramList;

        if (isHold) {
            paramList = requestHoldSe(soundId, resourceInfo->mEmitterName, pMeInfo, specificInfo,
                                      playInfo->mRequestKeeperName);
        } else {
            paramList = requestPlaySe(soundId, resourceInfo->mEmitterName, playInfo->mIsLoop, pMeInfo,
                                      specificInfo, playInfo->mRequestKeeperName);
        }

        if (paramList == nullptr) {
            continue;
        }

        if (firstParamList == nullptr) {
            firstParamList = paramList;
        }

        if (resourceInfo->mIsSetParamMin && resourceInfo->mParamMin > input) {
            playNum++;
            continue;
        }

        if (resourceInfo->mVolume != nullptr) {
            paramList->setVolume(alSeDbFunction::calcLeapValue(resourceInfo->mVolume, input));
        }

        if (resourceInfo->mPitch != nullptr) {
            paramList->setPitch(alSeDbFunction::calcLeapValue(resourceInfo->mPitch, input));
        }

        if (resourceInfo->mTempo != nullptr) {
            paramList->setTempo(alSeDbFunction::calcLeapValue(resourceInfo->mTempo, input));
        }

        if (resourceInfo->mLocalVarNo >= 0) {
            paramList->setLocalVariable(static_cast<s32>(input), resourceInfo->mLocalVarNo);
        }

        if (resourceInfo->mLfeSend > 0.0f) {
            paramList->setSpeakerVolumeLfeCenter(resourceInfo->mLfeSend, 0.0f);
        }

        playNum++;
    }

    if (playNum < 1 && !isTry) {
        return nullptr;
    }

    return firstParamList;
}

SePlayParamList* SeKeeper::requestPlaySeFromNameGetParamList(const char* pName, MeInfo* pMeInfo, bool isHold) {
    SePlayParamList* firstParamList = nullptr;

    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return firstParamList;
    }

    const SePlayInfo* playInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (playInfo == nullptr || playInfo->mResourceInfoList == nullptr) {
        return firstParamList;
    }

    s32 resourceNum = playInfo->mResourceInfoList->getInfoNum();

    if (resourceNum < 1) {
        return firstParamList;
    }

    if (isHold) {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* resourceInfo =
                playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(i) : nullptr;
            if (resourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* specificInfo =
                resourceInfo->mSpecificInfo != nullptr ? resourceInfo->mSpecificInfo : &sDefaultSpecificInfo;
            SePlayParamList* paramList = requestHoldSe(resourceInfo->mSoundId, resourceInfo->mEmitterName, pMeInfo,
                                                       specificInfo, playInfo->mRequestKeeperName);
            if (paramList != nullptr && firstParamList == nullptr) {
                firstParamList = paramList;
            }
        }
    } else {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* resourceInfo =
                playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(i) : nullptr;
            if (resourceInfo == nullptr) {
                continue;
            }

            const SeResourceSpecificInfo* specificInfo =
                resourceInfo->mSpecificInfo != nullptr ? resourceInfo->mSpecificInfo : &sDefaultSpecificInfo;
            SePlayParamList* paramList =
                requestPlaySe(resourceInfo->mSoundId, resourceInfo->mEmitterName, playInfo->mIsLoop, pMeInfo,
                              specificInfo, playInfo->mRequestKeeperName);
            if (paramList != nullptr && firstParamList == nullptr) {
                firstParamList = paramList;
            }
        }
    }

    return firstParamList;
}

void SeKeeper::stopSe(u32 id, s32 fadeFrames, const char* pEmitterName, const char* pPlayName) {
    if (mUserInfo == nullptr) {
        return;
    }

    if (pPlayName == nullptr) {
        pPlayName = mPlayName;
    }

    mSeDirector->stop(id, mEmitterHolder->findEmitter(pEmitterName)->getSeSource(), fadeFrames, pPlayName);
}

void SeKeeper::stopAllSeFromName(const char* pName, s32 fadeFrames, const char* pPlayName, bool isAll) {
    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return;
    }

    const SePlayInfo* playInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (playInfo == nullptr) {
        return;
    }

    if (fadeFrames <= 0) {
        fadeFrames = SePlayInfo::USE_DEFAULT_FADE_OUT_FRAME_NUM == playInfo->mFadeOutFrameNum ?
                         0 :
                         playInfo->mFadeOutFrameNum;
    }

    if (playInfo->mResourceInfoList == nullptr) {
        return;
    }

    s32 resourceNum = playInfo->mResourceInfoList->getInfoNum();

    if (resourceNum < 1) {
        return;
    }

    if (isAll) {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* resourceInfo = playInfo->mResourceInfoList->getInfoDirect(i);
            const char* emitterName = resourceInfo->mEmitterName;
            u32 soundId = resourceInfo->mSoundId;

            if (mEmitterHolder->findEmitter(emitterName) != nullptr) {
                mSeDirector->stopAllId(soundId, mEmitterHolder->findEmitter(emitterName)->getSeSource(), fadeFrames,
                                       pPlayName);
            }
        }
    } else {
        for (s32 i = 0; i < resourceNum; i++) {
            const SeResourceInfo* resourceInfo = playInfo->mResourceInfoList->getInfoDirect(i);
            const char* emitterName = resourceInfo->mEmitterName;
            u32 soundId = resourceInfo->mSoundId;

            if (mEmitterHolder->findEmitter(emitterName) != nullptr) {
                mSeDirector->stop(soundId, mEmitterHolder->findEmitter(emitterName)->getSeSource(), fadeFrames,
                                  pPlayName);
            }
        }
    }
}

void SeKeeper::stopSe(const char* pName) {
    stopSeFromName(pName);
}

void SeKeeper::stopSeFromName(const char* pName) {
    if (pName == nullptr || mUserInfo == nullptr || mUserInfo->mPlayInfoList == nullptr) {
        return;
    }

    const SePlayInfo* playInfo = mUserInfo->mPlayInfoList->tryFindInfo(pName);

    if (playInfo == nullptr) {
        return;
    }

    s32 fadeOutFrames = playInfo->mFadeOutFrameNum;
    s32 fadeFrames = SePlayInfo::USE_DEFAULT_FADE_OUT_FRAME_NUM == fadeOutFrames ? 0 : fadeOutFrames;

    if (playInfo->mResourceInfoList == nullptr) {
        return;
    }

    s32 resourceNum = playInfo->mResourceInfoList->getInfoNum();

    for (s32 i = 0; i < resourceNum; i++) {
        const SeResourceInfo* resourceInfo = playInfo->mResourceInfoList->getInfoDirect(i);
        const char* emitterName = resourceInfo->mEmitterName;
        u32 soundId = resourceInfo->mSoundId;

        if (mEmitterHolder->findEmitter(emitterName) != nullptr) {
            const char* playName = playInfo->mRequestKeeperName;

            if (playName == nullptr) {
                playName = mPlayName;
            }

            mSeDirector->stop(soundId, mEmitterHolder->findEmitter(emitterName)->getSeSource(), fadeFrames,
                              playName);
        }
    }
}

void SeKeeper::stopAll(s32 fadeFrames) {
    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->stopAllFromSource(alSeFunction::getSeSource(mEmitterHolder, i), fadeFrames);
    }
}

void SeKeeper::activate() {
    mIsActive = true;

    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->reactivateSeFromSource(mEmitterHolder->getEmitter(i)->getSeSource());
    }
}

void SeKeeper::deactivate(bool isClipped) {
    mIsActive = false;

    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        mSeDirector->deactivateSeFromSource(alSeFunction::getSeSource(mEmitterHolder, i), 0, isClipped);
    }
}

void SeKeeper::resetVelocity() {
    if (mUserInfo != nullptr) {
        mEmitterHolder->resetVelocity();
    }
}

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

void SeKeeper::setSeqLocalVariableDefault(s32 index, s32 value) {
    s32 slot;

    if (mSeqLocalVariables[0]->index == index) {
        slot = 0;
    } else if (mSeqLocalVariables[1]->index == index) {
        slot = 1;
    } else if (mSeqLocalVariables[2]->index == index) {
        slot = 2;
    } else if (mSeqLocalVariables[3]->index == index) {
        slot = 3;
    } else if (mSeqLocalVariables[0]->index < 0) {
        slot = 0;
    } else if (mSeqLocalVariables[1]->index < 0) {
        slot = 1;
    } else if (mSeqLocalVariables[2]->index < 0) {
        slot = 2;
    } else if (mSeqLocalVariables[3]->index < 0) {
        slot = 3;
    } else {
        return;
    }

    SeqLocalVariableDefault* variable = mSeqLocalVariables[slot];
    variable->value = value;
    mSeqLocalVariables[slot]->index = index;
}

void SeKeeper::setBiquadFilterDefault(s32 type, f32 value) {
    mBiquadFilter->type = type;
    mBiquadFilter->value = value;
}

void SeKeeper::setSeSourceVolume(f32 volume) {
    for (s32 i = 0; i < mEmitterHolder->getEmitterNum(); i++) {
        alSeFunction::getSeSource(mEmitterHolder, i)->setVolume(volume);
    }
}

void SeKeeper::loadSe(IAudioResourceLoader* pLoader) {
    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < (mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->getInfoNum() : 0); i++) {
        const SePlayInfo* playInfo =
            mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->tryGetInfo(i) : nullptr;
        if (playInfo == nullptr) {
            continue;
        }

        for (s32 j = 0;
             j < (playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->getInfoNum() : 0); j++) {
            const SeResourceInfo* resourceInfo =
                playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(j) : nullptr;
            if (resourceInfo == nullptr) {
                continue;
            }

            pLoader->loadSoundItem(resourceInfo->mSoundId, -1);
        }
    }
}

void SeKeeper::verifySe(SeadAudioPlayer* pPlayer, const char* pName) {
    if (mUserInfo == nullptr) {
        return;
    }

    for (s32 i = 0; i < (mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->getInfoNum() : 0); i++) {
        const SePlayInfo* playInfo =
            mUserInfo->mPlayInfoList != nullptr ? mUserInfo->mPlayInfoList->tryGetInfo(i) : nullptr;
        if (playInfo == nullptr) {
            continue;
        }

        for (s32 j = 0;
             j < (playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->getInfoNum() : 0); j++) {
            const SeResourceInfo* resourceInfo =
                playInfo->mResourceInfoList != nullptr ? playInfo->mResourceInfoList->tryGetInfo(j) : nullptr;
            if (resourceInfo == nullptr) {
                continue;
            }

            pPlayer->getSoundDataMgr()->IsDataLoaded(resourceInfo->mSoundId, -1);
        }
    }
}
}  // namespace al
