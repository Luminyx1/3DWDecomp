#include <attributes.h>
#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Library/Se/Project/SeKeeperInternal.hpp"
#include "Library/Se/Project/ISeModifier.hpp"
#include "Project/Se/SeEmitter.hpp"
#include "Project/Se/SeEmitterHolder.hpp"
#include "Project/Base/StringUtil.hpp"

#include "Library/Model/ModelShapeUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Info/SeSource.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * @brief Creates the SE source of an emitter.
 * @param pInfo Audio system information.
 * @param pEmitterInfo Emitter information.
 * @param pModelKeeper Model keeper of the owner, used for the emitter joint.
 * @param pPose Pose of the owner.
 * @param isUseModel Whether the default source is a 3D point instead of an ambient source.
 */
SeEmitter::SeEmitter(AudioSystemInfo* pInfo, const SeEmitterInfo* pEmitterInfo,
                     const ModelKeeper* pModelKeeper, SeSourcePose* pPose, bool isUseModel)
    : mEmitterInfo(pEmitterInfo) {
    const sead::Matrix34f* pMtx = nullptr;

    if (pEmitterInfo->mJointName != nullptr) {
        if (pModelKeeper != nullptr) {
            pMtx = getJointMtxPtr(pModelKeeper, pEmitterInfo->mJointName);
        }
    } else {
        SeSourcePose3DMtxBase* pMtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (pMtxPose != nullptr) {
            pMtx = pMtxPose->get3DMtxPtr();
        }
    }

    const sead::Vector3f* pOffset = mEmitterInfo->mOffset;

    if (pOffset != nullptr) {
        if (pMtx != nullptr) {
            pPose = new SeSourcePose3DMtxOffsetPtr(pMtx, pOffset);
        }
    } else if (pMtx != nullptr) {
        pPose = new SeSourcePose3DMtxPtr(pMtx);
    }

    const SeSoundSourceInfo* pSourceInfo = mEmitterInfo->mSoundSourceInfo;
    const char* pSourceName = isUseModel ? "３Ｄ点音源" : "環境音源";

    if (pSourceInfo != nullptr) {
        pSourceName = pSourceInfo->mName;
    }

    SeSource* pSource = nullptr;

    if (alSeFunction::isSoundSourceAmbient(pSourceName)) {
        pSource = new SeSourceAmbient(pInfo);
    } else if (alSeFunction::isSoundSource3DPoint(pSourceName)) {
        pSource = new SeSource3DPoint(sead::DynamicCast<SeSourcePose3D>(pPose), pInfo);
    } else if (alSeFunction::isSoundSource3DSphere(pSourceName)) {
        pSource =
            new SeSource3DSphere(sead::DynamicCast<SeSourcePose3D>(pPose),
                                 &static_cast<const SeSoundSourceInfo3DSphere*>(pSourceInfo)->mRadius, pInfo);
    } else if (alSeFunction::isSoundSource3DVector(pSourceName)) {
        SeSourcePose3DMtxBase* pMtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (pSourceInfo != nullptr && pMtxPose != nullptr) {
            pSource = new SeSource3DLine(
                pMtxPose, &static_cast<const SeSoundSourceInfo3DVector*>(pSourceInfo)->mVector, pInfo);
        }
    } else if (alSeFunction::isSoundSource3DBox(pSourceName)) {
        SeSourcePose3DMtxBase* pMtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (pSourceInfo != nullptr && pMtxPose != nullptr) {
            pSource =
                new SeSource3DPlaneRect(pMtxPose,
                                        reinterpret_cast<const sead::BoundBox2f*>(
                                            &static_cast<const SeSoundSourceInfo3DBox*>(pSourceInfo)->mMinX),
                                        pInfo);
        }
    } else if (alSeFunction::isSoundSource3DRing(pSourceName)) {
        SeSourcePose3DMtxBase* pMtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (pSourceInfo != nullptr && pMtxPose != nullptr) {
            pSource = new SeSource3DRing(
                pMtxPose, &static_cast<const SeSoundSourceInfo3DRing*>(pSourceInfo)->mRadius, pInfo);
        }
    } else if (alSeFunction::isSoundSource3DCircle(pSourceName)) {
        SeSourcePose3DMtxBase* pMtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (pSourceInfo != nullptr && pMtxPose != nullptr) {
            const SeSoundSourceInfo3DCircle* pCircleInfo =
                static_cast<const SeSoundSourceInfo3DCircle*>(pSourceInfo);
            pSource =
                new SeSource3DCircle(pMtxPose, &pCircleInfo->mRadius, pInfo, pCircleInfo->mIsCircleRotated);
        }
    }

    if (pSource != nullptr) {
        pSource->init();
    }

    mSeSource = pSource;
}

/**
 * @brief Updates the silence counter of the source.
 * @return True if the source has been silent long enough.
 */
bool SeEmitter::update() {
    if (mSilentFrames < 1) {
        return true;
    }

    if (mSeSource->isPlayingSound()) {
        mSilentFrames = 15;
    } else if (mSilentFrames >= 0) {
        mSilentFrames--;
    }

    return false;
}

/**
 * @brief Restarts the silence counter.
 */
NOINLINE void SeEmitter::activate() { mSilentFrames = 15; }

/**
 * @brief Gets the emitter name.
 * @return Emitter name.
 */
NOINLINE const char* SeEmitter::getName() const { return mEmitterInfo->mName; }
} // namespace al

namespace al {
/**
 * @brief Finds an emitter by name.
 * @param pName Emitter name, or nullptr for the first emitter.
 * @return Found emitter, or nullptr.
 */
NOINLINE SeEmitter* SeEmitterHolder::findEmitter(const char* pName) const {
    if (pName == nullptr) {
        return mEmitters.unsafeAt(0);
    }

    for (s32 i = 0; i < mEmitters.size(); i++) {
        SeEmitter* pEmitter = mEmitters.unsafeAt(i);

        if (isEqualString(pEmitter->getName(), pName)) {
            return pEmitter;
        }
    }

    return nullptr;
}

/**
 * @brief Gets an emitter by index.
 * @param index Emitter index.
 * @return Emitter, or nullptr if the index is out of range.
 */
SeEmitter* SeEmitterHolder::getEmitter(s32 index) const {
    if (index < mEmitters.size()) {
        return mEmitters.unsafeAt(index);
    }

    return nullptr;
}

/**
 * @brief Resets the velocity of all emitter sources.
 */
void SeEmitterHolder::resetVelocity() {
    for (s32 i = 0; i < mEmitters.size(); i++) {
        mEmitters.unsafeAt(i)->getSeSource()->resetVelocity();
    }
}
} // namespace al

namespace al {
/**
 * @brief Constructs the keeper, its default parameters, and its source pose.
 * @param pInfo Non-null audio system information supplying the sound database.
 * @param pDirector Sound director receiving playback requests.
 * @param pUserName Optional null-terminated name used to find the sound user definition.
 * @param pTrans Optional position followed when no matrix is supplied.
 * @param pMtx Optional followed matrix, taking precedence over pTrans.
 * @param pModelKeeper Optional model keeper used to resolve emitter joints.
 * @param pPlayName Optional request-keeper name; nullptr selects the keeper default.
 */
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

/**
 * @brief Updates emitters while the keeper and emitter holder are active.
 */
void SeKeeper::update() {
    if (mIsActive && mUserInfo != nullptr && mEmitterHolder->isActive()) {
        mEmitterHolder->update();
    }
}

/**
 * @brief Requests playback for a sound identifier or named play definition.
 * @param id Sound identifier to request or stop.
 * @param pEmitterName Optional emitter name; nullptr selects the first emitter.
 * @param isHold Whether to issue a held sound request.
 * @param pMeInfo Optional music-effect information forwarded to the sound director.
 * @param pSpecificInfo Optional resource-specific settings; nullptr selects database settings or shared
 * defaults.
 * @param pPlayName Optional request-keeper name; nullptr selects the keeper default.
 * @return Resulting parameter list, or nullptr when no request succeeds.
 */
SePlayParamList* SeKeeper::requestPlaySe(u32 id, const char* pEmitterName, bool isHold, MeInfo* pMeInfo,
                                         const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName) {
    SeEmitter* pEmitter = mEmitterHolder->findEmitter(pEmitterName);
    mEmitterHolder->setIsActive(true);
    pEmitter->activate();
    u32 soundId = id;

    if (mModifier != nullptr) {
        soundId = mModifier->modifySoundId(id);

        if (soundId == AudioConst::SOUND_ID_INVALID) {
            return nullptr;
        }
    }

    if (pSpecificInfo == nullptr) {
        pSpecificInfo = detail::findSpecificInfo(mSeDataBase, id);
    }

    if (pPlayName == nullptr) {
        pPlayName = mPlayName;
    }

    SePlayParamList* pParamList =
        mSeDirector->addRequest(soundId, pEmitter->getSeSource(), isHold, pSpecificInfo, pMeInfo,
                                mMaterialName, calcWaterState(), mIsBeyondWall, pPlayName);
    if (pParamList == nullptr) {
        return pParamList;
    }

    if (alSoundNameUtil::getSoundType(soundId, false) == 1) {
        applyKeeperParamsToParamList(pParamList, soundId, false);

        if (mModifier != nullptr) {
            mModifier->modifyStartParam(soundId, pParamList);
        }
    }

    return pParamList;
}

template s32 AudioInfoList<SeEmitterInfo>::getInfoNum() const;
} // namespace al
