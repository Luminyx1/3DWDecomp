#include "Library/Se/Project/SeRequestKeeper.hpp"
#include "Library/Se/Project/SeRequest.hpp"
#include "Library/Se/Project/SeCategory.hpp"
#include "Library/Se/Project/SePlayParamList.hpp"
#include "Library/Se/Info/SeSource.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/Audio/Sound/SoundHandle.hpp"
#include <nn/atk/atk_WaveSoundHandle.h>
#include <attributes.h>
#include "Library/Se/Project/InactiveSeListHolder.hpp"
#include "Library/Se/Function/SeWaitingListKeeper.hpp"
#include "Library/Se/Function/SeVolumeCtrl.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Library/Se/Project/SeMaterialInfoKeeper.hpp"

namespace al {
/** @brief Allocates reusable playback parameters, volume, and a sound handle for an empty request. */
NOINLINE SeRequest::SeRequest()
    : mSoundId(AudioConst::SOUND_ID_INVALID), mSpecificInfo(nullptr), mState(0), mPlayOrder(2), mPlayCount(0),
      mDistance(10000000.0f), mLifeTime(-1), mIsLoop(false), mIsPausedByDistance(false), mHandle(nullptr),
      mSource(nullptr), mParamList(nullptr), mMixVolume(new AudioMixVolume), mDelayMultiplier(-1),
      mVolume(1.0f) {
    mHandle = new AcLSoundHandle;
    mParamList = new SePlayParamList;
}

/** @brief Resets playback state and detaches the sound while retaining allocated request resources. */
void SeRequest::clear() {
    mSoundId = AudioConst::SOUND_ID_INVALID;
    mSpecificInfo = nullptr;
    mState = 0;
    mPlayOrder = 2;
    mPlayCount = 0;
    mDistance = 10000000.0f;
    mLifeTime = -1;
    mIsLoop = false;
    mIsPausedByDistance = false;
    if (mHandle->isAttachedSound()) {
        mHandle->detachSound();
    }
    mSource = nullptr;
    mParamList->reset();
    mDelayMultiplier = -1;
    mVolume = 1.0f;
    mMixVolume->resetLink();
}

/**
 * @brief Stops the sound and makes its request available for reuse.
 * @param fadeFrames Number of frames over which the sound fades out.
 */
inline void SeRequest::stopAndClear(s32 fadeFrames) {
    mHandle->stop(fadeFrames);
    clear();
}

/**
 * @brief Stops the sound and releases its request state.
 * @param fadeFrames Duration of the stop fade in frames.
 */
NOINLINE void SeRequest::stop(s32 fadeFrames) { stopAndClear(fadeFrames); }

/**
 * @brief Stops a request belonging to a source when either sound identifier matches.
 * @param soundId Current or original resource identifier to stop.
 * @param pSource Source whose request is eligible for stopping.
 * @param fadeFrames Number of fade-out frames.
 * @return True when this request was stopped.
 */
NOINLINE bool SeRequest::stopIfEqualIdOrOriginalId(u32 soundId, SeSource* pSource, s32 fadeFrames) {
    if (mSoundId == AudioConst::SOUND_ID_INVALID) {
        return false;
    }
    if (mSource != pSource) {
        return false;
    }
    if (mSoundId == soundId || calcOriginalId() == soundId) {
        stopAndClear(fadeFrames);
        return true;
    }
    return false;
}

/** @brief Gets the resource identifier before material substitution. @return Original or current sound ID. */
inline u32 SeRequest::calcOriginalId() const {
    u32 soundId = mSoundId;
    if (mSpecificInfo != nullptr && mSpecificInfo->mSoundId != AudioConst::SOUND_ID_INVALID) {
        soundId = mSpecificInfo->mSoundId;
    }
    return soundId;
}

/** @brief Gets the pre-substitution resource identifier. @return Original or current sound identifier. */
NOINLINE u32 SeRequest::getOriginalId() const { return calcOriginalId(); }

/**
 * @brief Reuses this request for a new sound.
 * @param soundId Valid archive sound identifier.
 * @param pSource Source responsible for starting and positioning the sound; must not be null.
 * @param isLoop Whether failed starts can remain paused for a later retry.
 * @param isHold Whether the request expires unless its three-frame lifetime is extended.
 * @param pSpecificInfo Resource-specific playback settings retained by the request.
 * @return Parameter list that the caller can fill before playback.
 */
NOINLINE SePlayParamList* SeRequest::setNew(u32 soundId, SeSource* pSource, bool isLoop, bool isHold,
                                            const SeResourceSpecificInfo* pSpecificInfo) {
    clear();
    mState = 1;
    mSource = pSource;
    mSoundId = soundId;
    mPlayCount = 0;
    mIsLoop = isLoop;
    mSpecificInfo = pSpecificInfo;
    mIsPausedByDistance = false;
    mLifeTime = isHold ? 3 : -1;
    return mParamList;
}

/** @brief Stops an attached sound immediately and marks its request paused. */
void SeRequest::pause() {
    if (mHandle->isAttachedSound()) {
        mHandle->stop(0);
    }
    verifyData();
    mState = 3;
}

/**
 * @brief Checks the invariants of an empty or active sound request.
 * @return True when stored fields and the attached sound agree with the request state.
 */
NOINLINE bool SeRequest::verifyData() const {
    bool result = true;
    if (mSoundId == AudioConst::SOUND_ID_INVALID) {
        if (result) {
            result &= mSource == nullptr;
        }
        if (result) {
            result &= mState == 0;
        }
        if (result) {
            result &= mDistance == 10000000.0f;
        }
        if (result) {
            result &= mPlayOrder == 2;
        }
        if (result) {
            result &= mPlayCount == 0;
        }
        if (result) {
            result &= mLifeTime == -1;
        }
        if (result) {
            result &= mHandle != nullptr;
        }
        if (result) {
            result &= mParamList != nullptr;
        }
        if (result) {
            result &= mSpecificInfo == nullptr;
        }
        if (result) {
            result &= !mHandle->isAttachedSound();
        }
    } else {
        if (result) {
            result &= mSource != nullptr;
        }
        if (result) {
            result &= mState != 0;
        }
        if (result) {
            result &= mDistance != 10000000.0f;
        }
        if (result) {
            result &= mHandle != nullptr;
        }
        if (result) {
            result &= mParamList != nullptr;
        }
        if (result) {
            result &= mSpecificInfo != nullptr;
        }
        if (result) {
            result &= (!mHandle->isAttachedSound() || mHandle->GetId() == mSoundId);
        }
    }
    return result;
}

/**
 * @brief Applies or releases a system pause on an attached sound.
 * @param isPause True to pause; false to resume unless the request itself remains paused.
 * @param fadeFrames Duration of the pause or resume fade in frames.
 */
NOINLINE void SeRequest::tryPauseBySystem(bool isPause, u32 fadeFrames) {
    if (!mHandle->isAttachedSound()) {
        return;
    }
    if (isPause) {
        mHandle->pause(fadeFrames);
    } else if (mState != 3) {
        mHandle->unpause(fadeFrames);
    }
}

/**
 * @brief Resumes an attached sound or asks the source to start it.
 * @param isAfterGoal Unused in this version of the playback-start implementation.
 */
void SeRequest::playStart(bool isAfterGoal) {
    if (mHandle->isAttachedSound()) {
        mHandle->unpause(0);
    } else {
        mSource->startSound(mHandle, mSoundId, nullptr);
        if (mHandle->isAttachedSound()) {
            mHandle->SetOutputLine(1);
        }
    }
    if (mHandle->isAttachedSound()) {
        mState = 2;
        verifyData();
    } else if (mIsLoop) {
        pause();
    } else {
        clear();
    }
}

/** @brief Extends a held request's lifetime to three frames. @return Parameters for the next update. */
NOINLINE SePlayParamList* SeRequest::extendHold() {
    mLifeTime = 3;
    return mParamList;
}

/** @brief Marks this request as waiting for its scheduled start. */
void SeRequest::setStateWaitForStart() { mState = 4; }

/** @brief Detaches the current sound and clears the request without stopping playback. */
void SeRequest::releaseHandle() {
    if (mHandle->isAttachedSound()) {
        mHandle->detachSound();
    }
    clear();
}

/** @brief Tests whether the attached sound supports a wave-sound handle. @return True for a wave sound. */
bool SeRequest::isWaveSound() const {
    if (mHandle != nullptr && mHandle->isAttachedSound()) {
        nn::atk::WaveSoundHandle handle(mHandle);
        if (handle.IsAttachedSound()) {
            return true;
        }
    }
    return false;
}

/** @brief Tests whether a playing request has lost its sound. @return True after playback has finished. */
bool SeRequest::isFinishedPlaying() const { return mState == 2 && !mHandle->isAttachedSound(); }

/** @brief Pauses a request because its source is outside the audible distance. */
void SeRequest::pauseByDistanceSe() {
    mIsPausedByDistance = true;
    pause();
}

/** @brief Clears the distance-pause flag so the request can be reconsidered for playback. */
void SeRequest::unpauseByDistanceSe() { mIsPausedByDistance = false; }

/** @brief Tests whether the request still owns a sound handle attachment. @return True when attached. */
bool SeRequest::isAttachedSound() const { return mHandle->isAttachedSound(); }

/** @brief Advances a non-paused request's hold lifetime and stops it when that lifetime reaches zero. */
void SeRequest::updateLifeTime() {
    if (mState == 3) {
        return;
    }
    const s32 nextLifeTime = mLifeTime - 1;
    if (nextLifeTime >= 0) {
        mLifeTime = nextLifeTime;
    }
    if (mLifeTime == 0) {
        stopAndClear(0);
    }
}

/** @brief Updates the source of a request whose sound identifier is valid. */
void SeRequest::updateSeSource() {
    if (mSoundId != AudioConst::SOUND_ID_INVALID) {
        mSource->update();
    }
}

/** @brief Increments the playback counter unless the request is invalid, paused, or waiting. */
void SeRequest::updatePlayCount() {
    if (mState == 3 || mState == 4) {
        return;
    }
    if (mSoundId == AudioConst::SOUND_ID_INVALID) {
        return;
    }
    ++mPlayCount;
}

/**
 * @brief Applies queued sound parameters and consumes the parameter list.
 * @param isAfterGoal Unused; goal-dependent volume is handled separately.
 * @param volume Unused in this implementation.
 * @param distance Unused in this implementation.
 */
void SeRequest::applyParamToRequest(bool isAfterGoal, f32 volume, f32 distance) {
    if (!mHandle->isAttachedSound()) {
        return;
    }
    f32 mulVolume = 1.0f;
    bool hasVolume = false;
    bool hasSpeakerVolume = false;
    f32 lfe = 0.0f, center = 0.0f, frontLeft = 0.0f, frontRight = 0.0f, rearLeft = 0.0f, rearRight = 0.0f;
    for (s32 i = 0; i < SePlayParamList::cRecordCount; ++i) {
        const SePlayParam* pParam = mParamList->getParam(i);
        switch (pParam->getType()) {
        case 1:
            mulVolume *= pParam->getValue();
            hasVolume = true;
            break;
        case 2: {
            const f32 value = pParam->getValue();
            mHandle->setPitch(value);
            break;
        }
        case 3: {
            const f32 value = pParam->getValue();
            mHandle->setSeqTempoRatio(value);
            break;
        }
        case 4:
            mHandle->writeSeqLocalVariable(static_cast<s32>(pParam->getSecondary()),
                                           static_cast<s16>(pParam->getValue()));
            break;
        case 5: {
            const s16 value = static_cast<s16>(pParam->getValue());
            const s32 index = static_cast<s32>(pParam->getSecondary());
            AcLSoundHandlePlatform::writeSeqGlobalVariable(index, value);
            break;
        }
        case 6: {
            const f32 value = pParam->getValue();
            mHandle->setLpfFreq(value);
            break;
        }
        case 7: {
            const f32 value = pParam->getValue();
            const f32 type = pParam->getSecondary();
            mHandle->setBiquadFilter(static_cast<s32>(type), value);
            break;
        }
        case 8:
            lfe = pParam->getValue();
            center = pParam->getSecondary();
            hasSpeakerVolume = true;
            break;
        case 9:
            frontLeft = pParam->getValue();
            frontRight = pParam->getSecondary();
            hasSpeakerVolume = true;
            break;
        case 10:
            rearLeft = pParam->getValue();
            rearRight = pParam->getSecondary();
            hasSpeakerVolume = true;
            break;
        }
    }
    if (hasVolume) {
        mVolume = mulVolume;
    }
    if (hasSpeakerVolume) {
        mHandle->setAllOutputDeviceSpeakerVolume(frontLeft, frontRight, rearLeft, rearRight, center, lfe);
    }
    if (mParamList->hasOutputLine()) {
        mHandle->SetOutputLine(mParamList->getOutputLine());
    }
    mParamList->reset();
}

/**
 * @brief Applies source, request, category, and linked volume multipliers to the sound.
 * @param isAfterGoal Unused in this version of the volume calculation.
 * @param volume Linear multiplier supplied by the request keeper.
 */
void SeRequest::applyVolume(bool isAfterGoal, f32 volume) {
    if (mSource == nullptr) {
        return;
    }
    f32 mixRatio = calcDecibelToRatio(mMixVolume->calcLinkedVolumeDecibel());
    mHandle->setVolume(mixRatio * (mSource->getVolume() * mVolume * volume), 0);
}

/**
 * @brief Stores listener distance with a small ordering bias for request prioritization.
 * @param rListenerPos Listener position in world coordinates.
 * @param order Request ordering index; contributes one hundredth of a unit per index.
 * @return True when the source uses spatial calculations.
 */
bool SeRequest::calcDistance(sead::Vector3f& rListenerPos, s32 order) {
    f32 distance = 0.0f;
    if (mSource->isCalc3D()) {
        distance = (*mSource->getPosition() - rListenerPos).length();
    }
    mDistance = static_cast<f32>(order) / 100.0f + distance;
    return mSource->isCalc3D();
}

/** @brief Gets the source's current position. @return Position pointer supplied by the source. */
const sead::Vector3f* SeRequest::getPosition() const { return mSource->getPosition(); }

/**
 * @brief Queues an additional volume multiplier.
 * @param volume Linear multiplier combined with other queued volume values.
 */
void SeRequest::setMulParamVolume(f32 volume) { mParamList->setMulVolume(volume); }

/**
 * @brief Queues a low-pass filter frequency.
 * @param freq Filter frequency value forwarded without clamping.
 */
void SeRequest::setParamLpfFreq(f32 freq) { mParamList->setLpfFreq(freq); }

/**
 * @brief Allocates the reusable request pool and its inactive, delayed, and volume controllers.
 * @param pMgr Shared spatial-audio manager.
 * @param pPlayer Archive player used for sound-category queries.
 * @param pName Keeper name retained for later selection.
 * @param requestNum Number of reusable requests to allocate; must be nonnegative.
 * @param distance Distance limit retained by the keeper.
 */
SeRequestKeeper::SeRequestKeeper(SeadAudio3DMgr* pMgr, SeadAudioPlayer* pPlayer, const char* pName,
                                 s32 requestNum, f32 distance)
    : mRequestNum(requestNum), mName(pName), mDistance(distance) {
    mActiveRequests.initOffset(SeRequest::getNodeOffset());
    mRequests.allocBuffer(requestNum, nullptr);
    for (s32 i = 0; i < mRequestNum; ++i) {
        mRequests.pushBack(new SeRequest);
    }
    mInactiveRequests = new InactiveSeListHolder;
    mWaitingRequests = new SeWaitingListKeeper;
    mVolumeCtrl = new SeVolumeCtrl(&mActiveRequests, pPlayer, mName);
    mAudio3DMgr = pMgr;
    mPlayer = pPlayer;
}

/**
 * @brief Allocates a free request and adds it to active playback.
 * @param soundId Archive identifier; invalid identifiers are rejected.
 * @param pSource Source responsible for playback; must not be null.
 * @param isLoop Whether the sound should remain available for restart after a failed start.
 * @param pSpecificInfo Playback settings; must not be null when CM filtering is enabled.
 * @param pMixVolume Optional volume controller to link to this request.
 * @return Writable playback parameters, or nullptr when the request is rejected or the pool is full.
 */
NOINLINE SePlayParamList* SeRequestKeeper::addRequest(u32 soundId, SeSource* pSource, bool isLoop,
                                                      const SeResourceSpecificInfo* pSpecificInfo,
                                                      const AudioMixVolume* pMixVolume) {
    if (!mIsActive || soundId == AudioConst::SOUND_ID_INVALID) {
        return nullptr;
    }
    if (mIsExcludeCmNgSe && pSpecificInfo->mIsCmNg) {
        return nullptr;
    }
    for (s32 i = 0; i < mRequestNum; ++i) {
        if (mRequests.unsafeAt(i)->getSoundId() == AudioConst::SOUND_ID_INVALID) {
            SePlayParamList* pParams =
                mRequests.at(i)->setNew(soundId, pSource, isLoop, false, pSpecificInfo);
            mActiveRequests.pushBack(mRequests.at(i));
            if (pMixVolume != nullptr) {
                mRequests.unsafeAt(i)->getMixVolume()->linkTo(pMixVolume);
            }
            return pParams;
        }
    }
    return nullptr;
}

/**
 * @brief Extends an existing held sound or allocates a request with a three-frame lifetime.
 * @param soundId Archive identifier; invalid identifiers are rejected.
 * @param pSource Source whose held request is searched for or created.
 * @param pSpecificInfo Resource playback settings; required when CM filtering is enabled.
 * @param pMixVolume Optional linked volume for a newly allocated request.
 * @return Writable playback parameters, or nullptr if no eligible request can be obtained.
 */
SePlayParamList* SeRequestKeeper::addHoldRequest(u32 soundId, SeSource* pSource,
                                                 const SeResourceSpecificInfo* pSpecificInfo,
                                                 const AudioMixVolume* pMixVolume) {
    if (!mIsActive || soundId == AudioConst::SOUND_ID_INVALID) {
        return nullptr;
    }
    if (mIsExcludeCmNgSe && pSpecificInfo->mIsCmNg) {
        return nullptr;
    }
    SePlayParamList* pParams = nullptr;
    const auto end = mActiveRequests.end();
    for (auto it = mActiveRequests.begin(); it != end; ++it) {
        if (it->getSoundId() != AudioConst::SOUND_ID_INVALID && it->getSoundId() == soundId &&
            it->getSource() == pSource) {
            pParams = it->extendHold();
            break;
        }
    }
    if (pParams != nullptr) {
        return pParams;
    }
    for (s32 i = 0; i < mRequestNum; ++i) {
        if (mRequests.unsafeAt(i)->getSoundId() == AudioConst::SOUND_ID_INVALID) {
            pParams = mRequests.at(i)->setNew(soundId, pSource, true, true, pSpecificInfo);
            mActiveRequests.pushBack(mRequests.at(i));
            if (pMixVolume != nullptr) {
                mRequests.unsafeAt(i)->getMixVolume()->linkTo(pMixVolume);
            }
            return pParams;
        }
    }
    return nullptr;
}

/**
 * @brief Inserts an already prepared request into the active list when the keeper is enabled.
 * @param pRequest Prepared request whose node must not already belong to a list.
 */
void SeRequestKeeper::addRequestDirect(SeRequest* pRequest) {
    if (mIsActive) {
        mActiveRequests.pushBack(pRequest);
    }
}

/**
 * @brief Stops and removes active requests matching a source and current or original sound ID.
 * @param soundId Sound identifier to match.
 * @param pSource Source whose matching sounds are stopped.
 * @param fadeFrames Duration of the stop fade in frames.
 */
void SeRequestKeeper::stop(u32 soundId, SeSource* pSource, u32 fadeFrames) {
    if (!mIsActive) {
        return;
    }
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->stopIfEqualIdOrOriginalId(soundId, pSource, fadeFrames)) {
            mActiveRequests.erase(&*it);
        }
    }
}

/** @brief Enables the keeper to accept and process requests. */
void SeRequestKeeper::activateSystem() { mIsActive = true; }

/** @brief Stops active sounds and prevents further requests until reactivated. */
void SeRequestKeeper::deactivateSystem() {
    if (mIsActive) {
        for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
            if (it->getSoundId() != AudioConst::SOUND_ID_INVALID) {
                stopAndRemove(&*it, 0);
            }
        }
    }
    mIsActive = false;
}

/**
 * @brief Selects a volume preset for this request keeper.
 * @param pName Name of the preset; unknown names leave the current setting unchanged.
 * @param fadeFrames Duration of the transition in frames.
 */
void SeRequestKeeper::setVolumeSetting(const char* pName, s32 fadeFrames) {
    mVolumeCtrl->setVolumeSetting(pName, static_cast<f32>(fadeFrames));
}

/** @brief Allocates 1024 reusable slots for inactive sounds. */
NOINLINE InactiveSeListHolder::InactiveSeListHolder() {
    mParams.allocBuffer(1024, nullptr);
    for (s32 i = 0; i < 1024; ++i) {
        mParams.pushBack(new InactiveSeParam);
    }
}

/**
 * @brief Retains an inactive sound in the first available slot.
 * @param soundId Identifier to retain; an invalid identifier denotes an empty slot.
 * @param pSource Source used to find this sound when it is reactivated.
 * @param pSpecificInfo Playback settings retained without copying.
 * @param pMixVolume Linked volume retained without taking ownership.
 */
NOINLINE void InactiveSeListHolder::addSe(u32 soundId, SeSource* pSource,
                                          const SeResourceSpecificInfo* pSpecificInfo,
                                          const AudioMixVolume* pMixVolume) {
    for (s32 i = 0; i < mParams.size(); ++i) {
        if (mParams.unsafeAt(i)->mSoundId == AudioConst::SOUND_ID_INVALID) {
            mParams.unsafeAt(i)->mSoundId = soundId;
            mParams.unsafeAt(i)->mSource = pSource;
            mParams.unsafeAt(i)->mSpecificInfo = pSpecificInfo;
            mParams.unsafeAt(i)->mMixVolume = pMixVolume;
            return;
        }
    }
}

/**
 * @brief Copies a retained sound's identifier and settings when its source matches.
 * @param pInfo Output record; its source and linked-volume fields are left unchanged.
 * @param index Slot index in the range [0, 1024).
 * @param pSource Source that the selected slot must reference.
 */
NOINLINE void InactiveSeListHolder::getInfoIfEqualSource(InactiveSeParam* pInfo, s32 index,
                                                         SeSource* pSource) const {
    if (mParams.unsafeAt(index)->mSource == pSource) {
        pInfo->mSoundId = mParams.unsafeAt(index)->mSoundId;
        pInfo->mSpecificInfo = mParams.unsafeAt(index)->mSpecificInfo;
    }
}

/**
 * @brief Frees one retained sound slot.
 * @param index Slot index in the range [0, 1024).
 */
NOINLINE void InactiveSeListHolder::clear(s32 index) { mParams.unsafeAt(index)->clear(); }

/**
 * @brief Frees the first retained slot belonging to a source.
 * @param pSource Source whose first retained sound is discarded.
 */
NOINLINE void InactiveSeListHolder::clearIfEqualSource(SeSource* pSource) {
    for (s32 i = 0; i < mParams.size(); ++i) {
        if (mParams.unsafeAt(i)->mSource == pSource) {
            mParams.unsafeAt(i)->clear();
            return;
        }
    }
}

/** @brief Frees all retained sound slots without deallocating their storage. */
void InactiveSeListHolder::clearAll() {
    for (s32 i = 0; i < mParams.size(); ++i) {
        mParams.unsafeAt(i)->clear();
    }
}

/**
 * @brief Stops an active request and removes its node.
 * @param pRequest Request currently linked to this keeper's active list.
 * @param fadeFrames Number of fade-out frames.
 */
inline void SeRequestKeeper::stopAndRemove(SeRequest* pRequest, u32 fadeFrames) {
    pRequest->stop(fadeFrames);
    mActiveRequests.erase(pRequest);
}
/**
 * @brief Stops looping sounds from a source and optionally retains indefinite loops.
 * @param pSource Source whose looping sounds are deactivated.
 * @param fadeFrames Number of fade-out frames.
 * @param isClipped True to retain indefinite loops for reactivation; false to discard a retained entry.
 */
void SeRequestKeeper::deactivateSeFromSource(SeSource* pSource, u32 fadeFrames, bool isClipped) {
    if (!mIsActive) {
        return;
    }
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->getSoundId() != AudioConst::SOUND_ID_INVALID && it->getSource() == pSource && it->isLoop()) {
            if (isClipped && it->getLifeTime() < 0) {
                mInactiveRequests->addSe(it->getSoundId(), pSource, it->getSpecificInfo(),
                                         it->getMixVolume()->getLinkedVolume());
            }
            stopAndRemove(&*it, fadeFrames);
        }
    }
    if (!isClipped) {
        mInactiveRequests->clearIfEqualSource(pSource);
    }
}
/**
 * @brief Restarts retained looping sounds belonging to a source.
 * @param pSource Source whose retained sounds should reenter active playback.
 */
void SeRequestKeeper::reactivateSeFromSource(SeSource* pSource) {
    if (!mIsActive) {
        return;
    }
    for (s32 i = 0; i < mInactiveRequests->getNum(); ++i) {
        InactiveSeParam info;
        mInactiveRequests->getInfoIfEqualSource(&info, i, pSource);
        if (info.mSoundId != AudioConst::SOUND_ID_INVALID) {
            addRequest(info.mSoundId, pSource, true, info.mSpecificInfo, info.mMixVolume);
            mInactiveRequests->clear(i);
        }
    }
}
/**
 * @brief Stops a source's active sounds and discards its first retained sound.
 * @param pSource Source whose sounds are stopped.
 * @param fadeFrames Number of fade-out frames.
 */
void SeRequestKeeper::stopAllFromSource(SeSource* pSource, u32 fadeFrames) {
    if (!mIsActive) {
        return;
    }
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->getSoundId() != AudioConst::SOUND_ID_INVALID && it->getSource() == pSource) {
            stopAndRemove(&*it, fadeFrames);
        }
    }
    mInactiveRequests->clearIfEqualSource(pSource);
}
/**
 * @brief Stops non-looping trigger sounds.
 * @param fadeFrames Number of fade-out frames.
 */
void SeRequestKeeper::stopAllTrigSe(u32 fadeFrames) {
    if (!mIsActive) {
        return;
    }
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->getSoundId() != AudioConst::SOUND_ID_INVALID && !it->isLoop()) {
            stopAndRemove(&*it, fadeFrames);
        }
    }
}
/**
 * @brief Stops active sounds except those named in an exclusion list.
 * @param fadeFrames Number of fade-out frames.
 * @param pExceptList Sound names to preserve; nullptr causes no sounds to be stopped.
 * @param exceptNum Number of names; zero causes no sounds to be stopped.
 */
void SeRequestKeeper::stopAllExceptList(u32 fadeFrames, const char** pExceptList, u32 exceptNum) {
    if (pExceptList == nullptr || exceptNum == 0) {
        return;
    }
    if (!mIsActive) {
        return;
    }
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        u32 soundId = it->getSoundId();
        if (soundId == AudioConst::SOUND_ID_INVALID) {
            continue;
        }
        bool isExcluded = false;
        for (u32 i = 0; i < exceptNum; ++i) {
            if (alSoundNameUtil::getSoundId(pExceptList[i], false) == soundId) {
                isExcluded = true;
                break;
            }
        }
        if (!isExcluded) {
            stopAndRemove(&*it, fadeFrames);
        }
    }
}

/**
 * @brief Tests whether a category should be interrupted for a camera demo.
 * @param it Iterator referring to an active request.
 * @param type Demo policy selecting categories that remain audible.
 * @return True when archive information permits interrupting the category.
 */
bool SeRequestKeeper::isStopCategoryForDemo(RequestList::robustIterator it, alSeFunction::DemoType type) {
    SoundInfo info;
    bool isStop = mPlayer->readSoundInfo(&info, it->getSoundId());
    if (!isStop) {
        return false;
    }
    {
        switch (type) {
        case 3:
            if (info.playerId == 0x4000005) {
                isStop = false;
            } else if (info.playerId == 0x4000001 || info.playerId == 0x4000006 ||
                       info.playerId == 0x4000007 || info.playerId == 0x4000008 ||
                       info.playerId == 0x400000a) {
                isStop = false;
            }
            break;
        case 2:
            if (info.playerId == 0x4000005) {
                isStop = false;
            }
            break;
        case 1:
            if (info.playerId == 0x4000001 || info.playerId == 0x4000006 || info.playerId == 0x4000007 ||
                info.playerId == 0x4000008 || info.playerId == 0x400000a) {
                isStop = false;
            }
            break;
        }
    }
    return isStop;
}
/**
 * @brief Identifies player-category requests that should stop during camera demos.
 * @param it Iterator referring to an active request.
 * @return True for player categories except the route-pipe movement loop.
 */
bool SeRequestKeeper::isCategoryPlayer(RequestList::robustIterator it) {
    SoundInfo info;
    if (!mPlayer->readSoundInfo(&info, it->getSoundId())) {
        return false;
    }
    if (info.playerId == 0x4000000 || info.playerId == 0x4000002) {
        u32 soundId = it->getSoundId();
        return alSoundNameUtil::getSoundId("SePmRouteDokanMoveLv", false) != soundId;
    }
    return false;
}
/**
 * @brief Stops eligible player and trigger sounds and pauses eligible loops for a camera demo.
 * @param type Demo policy selecting categories that remain audible.
 */
void SeRequestKeeper::stopSeForCameraDemo(alSeFunction::DemoType type) {
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->getSoundId() == AudioConst::SOUND_ID_INVALID || !isStopCategoryForDemo(it, type)) {
            continue;
        }
        if (isCategoryPlayer(it)) {
            stopAndRemove(&*it, 0);
        } else if (it->getSoundId() != AudioConst::SOUND_ID_INVALID && it->isLoop()) {
            it->tryPauseBySystem(true, 0);
        } else {
            stopAndRemove(&*it, 0);
        }
    }
}
/**
 * @brief Resumes looping sounds affected by a completed camera demo.
 * @param type Demo policy used when the sounds were paused.
 */
void SeRequestKeeper::startPausedSeFromCameraDemo(alSeFunction::DemoType type) {
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->getSoundId() != AudioConst::SOUND_ID_INVALID && it->isLoop() &&
            isStopCategoryForDemo(it, type)) {
            it->tryPauseBySystem(false, 0);
        }
    }
}
/**
 * @brief Sets or clears one named system-pause reason.
 * @param isPause New state of the selected pause reason.
 * @param pName System pause, demo, or system error reason; unknown names are ignored.
 */
void SeRequestKeeper::updatePauseFlag(bool isPause, const char* pName) {
    if (isEqualString(pName, "システムポーズ")) {
        mIsSystemPause = isPause;
    } else if (isEqualString(pName, "デモ")) {
        mIsDemoPause = isPause;
    } else if (isEqualString(pName, "システムエラー")) {
        mIsErrorPause = isPause;
    }
}
/**
 * @brief Updates a pause reason and forwards transitions to active sound handles.
 * @param isPause Whether the selected reason is becoming active.
 * @param pName Named pause reason; unknown names leave the flags unchanged.
 * @param fadeFrames Number of frames in the pause or resume fade.
 */
void SeRequestKeeper::pauseSystem(bool isPause, const char* pName, u32 fadeFrames) {
    if (!mIsActive) {
        return;
    }
    bool wasPaused = isSystemPaused();
    updatePauseFlag(isPause, pName);
    if (isSystemPaused() && wasPaused) {
        return;
    }
    if (!isSystemPaused() && !wasPaused) {
        return;
    }
    for (auto it = mActiveRequests.begin(); it != mActiveRequests.end(); ++it) {
        if (it->getSoundId() != AudioConst::SOUND_ID_INVALID) {
            it->tryPauseBySystem(isPause, fadeFrames);
        }
    }
}

namespace {
/**
 * @brief Identifies sounds that continue during the miss transition.
 * @param soundId Archive identifier of the sound being considered.
 * @return True for death, damage, and transition-feedback sounds retained by that transition.
 */
inline bool isMissTransitionSound(u32 soundId) {
    return alSoundNameUtil::getSoundId("SeSyDamageLast", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvFallDown", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvFallDownWater", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvDamageLast", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvDamageLastWater", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvDamageLava", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvDamagePoison", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvForceDie", false) == soundId ||
           alSoundNameUtil::getSoundId("SePmForceDieDummy", false) == soundId ||
           alSoundNameUtil::getSoundId("SePmForceDieLava", false) == soundId ||
           alSoundNameUtil::getSoundId("SePmForceDiePoison", false) == soundId ||
           alSoundNameUtil::getSoundId("SePvGigaDamageLast", false) == soundId ||
           alSoundNameUtil::getSoundId("SeSyGeneralWindowDecideS", false) == soundId ||
           alSoundNameUtil::getSoundId("SeNvKoopaJrPlayerDie", false) == soundId;
}
} // namespace
/**
 * @brief Stops active sounds with an optional miss-transition exclusion policy.
 * @param fadeFrames Number of fade-out frames.
 * @param pExceptName Policy name; only the miss policy preserves sounds, and nullptr preserves none.
 */
void SeRequestKeeper::stopAll(u32 fadeFrames, const char* pExceptName) {
    if (!mIsActive) {
        return;
    }
    if (pExceptName != nullptr && isEqualString(pExceptName, "ミス")) {
        for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
            u32 soundId = it->getSoundId();
            if (soundId != AudioConst::SOUND_ID_INVALID && !isMissTransitionSound(soundId)) {
                stopAndRemove(&*it, fadeFrames);
            }
        }
    } else {
        for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
            if (it->getSoundId() != AudioConst::SOUND_ID_INVALID) {
                stopAndRemove(&*it, fadeFrames);
            }
        }
    }
}
/**
 * @brief Replaces indefinite looping sounds when a source's material changes.
 * @param pSource Source whose material-dependent sounds are reconsidered.
 * @param pMaterialName New material name passed to the substitution lookup.
 * @param waterState Material-state selector for dry, wet, water, or single-player wet sounds.
 * @param pMaterialKeeper Material substitution service; must not be null.
 */
void SeRequestKeeper::notifiedUpdateMaterial(SeSource* pSource, const char* pMaterialName, s32 waterState,
                                             SeMaterialInfoKeeper* pMaterialKeeper) {
    if (!mIsActive) {
        return;
    }
    for (auto it = mActiveRequests.robustBegin(); it != mActiveRequests.robustEnd(); ++it) {
        if (it->getSoundId() == AudioConst::SOUND_ID_INVALID || !it->isLoop() || it->getSource() != pSource) {
            continue;
        }
        const SeResourceSpecificInfo* pSpecificInfo = it->getSpecificInfo();
        u32 soundId =
            pMaterialKeeper->findReplacedId(it->getOriginalId(), pMaterialName, waterState, pSpecificInfo);
        if (soundId != it->getSoundId() && it->getLifeTime() <= 0) {
            bool isLoop = it->getSoundId() != AudioConst::SOUND_ID_INVALID && it->isLoop();
            const AudioMixVolume* pMixVolume = it->getMixVolume()->getLinkedVolume();
            stopAndRemove(&*it, 0);
            addRequest(soundId, pSource, isLoop, pSpecificInfo, pMixVolume);
        }
    }
}
} // namespace al
