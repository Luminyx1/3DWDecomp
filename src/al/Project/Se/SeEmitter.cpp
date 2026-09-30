#include "Project/Se/SeEmitter.hpp"

#include "Library/Model/ModelShapeUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Info/SeSource.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Creates the SE source of an emitter.
 * @param pInfo Audio system information.
 * @param pEmitterInfo Emitter information.
 * @param pModelKeeper Model keeper of the owner, used for the emitter joint.
 * @param pPose Pose of the owner.
 * @param isUseModel Whether the default source is a 3D point instead of an ambient source.
 */
SeEmitter::SeEmitter(AudioSystemInfo* pInfo, const SeEmitterInfo* pEmitterInfo, const ModelKeeper* pModelKeeper,
                     SeSourcePose* pPose, bool isUseModel)
    : mEmitterInfo(pEmitterInfo) {
    const sead::Matrix34f* mtx = nullptr;

    if (pEmitterInfo->mJointName != nullptr) {
        if (pModelKeeper != nullptr) {
            mtx = getJointMtxPtr(pModelKeeper, pEmitterInfo->mJointName);
        }
    } else {
        SeSourcePose3DMtxBase* mtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (mtxPose != nullptr) {
            mtx = mtxPose->get3DMtxPtr();
        }
    }

    const sead::Vector3f* offset = mEmitterInfo->mOffset;

    if (offset != nullptr) {
        if (mtx != nullptr) {
            pPose = new SeSourcePose3DMtxOffsetPtr(mtx, offset);
        }
    } else if (mtx != nullptr) {
        pPose = new SeSourcePose3DMtxPtr(mtx);
    }

    const SeSoundSourceInfo* sourceInfo = mEmitterInfo->mSoundSourceInfo;
    const char* sourceName = isUseModel ? "３Ｄ点音源" : "環境音源";

    if (sourceInfo != nullptr) {
        sourceName = sourceInfo->mName;
    }

    SeSource* source = nullptr;

    if (alSeFunction::isSoundSourceAmbient(sourceName)) {
        source = new SeSourceAmbient(pInfo);
    } else if (alSeFunction::isSoundSource3DPoint(sourceName)) {
        source = new SeSource3DPoint(sead::DynamicCast<SeSourcePose3D>(pPose), pInfo);
    } else if (alSeFunction::isSoundSource3DSphere(sourceName)) {
        source = new SeSource3DSphere(sead::DynamicCast<SeSourcePose3D>(pPose),
                                      &static_cast<const SeSoundSourceInfo3DSphere*>(sourceInfo)->mRadius, pInfo);
    } else if (alSeFunction::isSoundSource3DVector(sourceName)) {
        SeSourcePose3DMtxBase* mtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (sourceInfo != nullptr && mtxPose != nullptr) {
            source = new SeSource3DLine(
                mtxPose, &static_cast<const SeSoundSourceInfo3DVector*>(sourceInfo)->mVector, pInfo);
        }
    } else if (alSeFunction::isSoundSource3DBox(sourceName)) {
        SeSourcePose3DMtxBase* mtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (sourceInfo != nullptr && mtxPose != nullptr) {
            source = new SeSource3DPlaneRect(mtxPose,
                                             reinterpret_cast<const sead::BoundBox2f*>(
                                                 &static_cast<const SeSoundSourceInfo3DBox*>(sourceInfo)->mMinX),
                                             pInfo);
        }
    } else if (alSeFunction::isSoundSource3DRing(sourceName)) {
        SeSourcePose3DMtxBase* mtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (sourceInfo != nullptr && mtxPose != nullptr) {
            source =
                new SeSource3DRing(mtxPose, &static_cast<const SeSoundSourceInfo3DRing*>(sourceInfo)->mRadius, pInfo);
        }
    } else if (alSeFunction::isSoundSource3DCircle(sourceName)) {
        SeSourcePose3DMtxBase* mtxPose = sead::DynamicCast<SeSourcePose3DMtxBase>(pPose);

        if (sourceInfo != nullptr && mtxPose != nullptr) {
            const SeSoundSourceInfo3DCircle* circleInfo = static_cast<const SeSoundSourceInfo3DCircle*>(sourceInfo);
            source = new SeSource3DCircle(mtxPose, &circleInfo->mRadius, pInfo, circleInfo->mIsCircleRotated);
        }
    }

    if (source != nullptr) {
        source->init();
    }

    mSeSource = source;
}

/**
 * Updates the silence counter of the source.
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
 * Restarts the silence counter.
 */
void SeEmitter::activate() {
    mSilentFrames = 15;
}

/**
 * Gets the emitter name.
 * @return Emitter name.
 */
const char* SeEmitter::getName() const {
    return mEmitterInfo->mName;
}
}  // namespace al
