#include "Project/Effect/EffectEmitter.hpp"

#include <ptcl/seadPtclSystem.h>

#include "Library/Effect/EffectSystemInfo.hpp"
#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"

namespace al {
/**
 * Creates an emitter for an effect resource, with a ring of handles for non-looping effects.
 * @param pSystemInfo Effect system info.
 * @param pResourceInfo Effect resource info.
 * @param handleNum Number of handles to cycle through.
 */
EffectEmitter::EffectEmitter(const EffectSystemInfo* pSystemInfo, EffectResourceInfo* pResourceInfo,
                             s32 handleNum)
    : mSystemInfo(pSystemInfo), mResourceInfo(pResourceInfo) {
    if (mResourceInfo->mName != nullptr &&
        mResourceInfo->mEmitterSetResourceInfo->mEmitterSetId < 0) {
        alEffectFunction::initResourceInfo(mSystemInfo, mResourceInfo);
    }

    if (handleNum >= 2 && !mResourceInfo->mEmitterSetResourceInfo->mIsLoop &&
        !mResourceInfo->mEmitterSetResourceInfo->mIsInfinity) {
        mHandleNum = handleNum;
        mHandles = new sead::ptcl::Handle*[handleNum];

        for (s32 i = 0; i < mHandleNum; i++) {
            mHandles[i] = new sead::ptcl::Handle();
        }

        mHandle = mHandles[mHandleIndex];
    }

    if (mHandles == nullptr) {
        mHandle = new sead::ptcl::Handle();
    }
}

/**
 * Sets the matrix the emitter follows.
 * @param mtxPtr Joint matrix pointer.
 */
void EffectEmitter::initMtxPtr(JointMtxPtr mtxPtr) {
    mJointMtxPtr = mtxPtr;
}

/**
 * Updates the matrix the emitter follows.
 * @param mtxPtr Joint matrix pointer.
 */
void EffectEmitter::updateMtxPtr(JointMtxPtr mtxPtr) {
    mJointMtxPtr = mtxPtr;
}

/**
 * Creates the emitter set at a matrix or a position.
 * @param pMtxPtr Matrix to create at, or nullptr to use the position.
 * @param pPos Position used when there is no matrix.
 * @param groupId Emitter group.
 * @param forceCalcFrame Frames to calculate right away.
 * @param userData User data stored in the emitter set.
 */
void EffectEmitter::createEmitter(const JointMtxPtr* pMtxPtr, const sead::Vector3f* pPos, s32 groupId,
                                  s32 forceCalcFrame, u64 userData) {
    mIsFirstFrame = true;
    mEmitFrame = mSystemInfo->_0;
    mForceCalcFrame = 0;

    if (mResourceInfo->mEmitterSetResourceInfo->mEmitterSetId < 0) {
        return;
    }

    if (mHandles != nullptr) {
        mHandleIndex = mHandleNum > mHandleIndex + 1 ? mHandleIndex + 1 : 0;
        mHandle = mHandles[mHandleIndex];
    }

    if (pMtxPtr != nullptr) {
        sead::Matrix34f mtx;
        pMtxPtr->copyTo(&mtx);

        if (!mSystemInfo->mPtclSystem->createEmitterSetID(
                mHandle, mtx, mResourceInfo->mEmitterSetResourceInfo->mEmitterSetId,
                mResourceInfo->mEmitterSetResourceInfo->mResourceId, groupId)) {
            return;
        }
    } else {
        if (!mSystemInfo->mPtclSystem->createEmitterSetID(
                mHandle, *pPos, mResourceInfo->mEmitterSetResourceInfo->mEmitterSetId,
                mResourceInfo->mEmitterSetResourceInfo->mResourceId, groupId)) {
            return;
        }
    }

    nn::vfx::EmitterSet* emitterSet = mHandle->GetEmitterSet();
    emitterSet->SetUserData(userData);

    if (forceCalcFrame > 0 && emitterSet != nullptr) {
        emitterSet->ForceCalculate(forceCalcFrame);
        mForceCalcFrame = forceCalcFrame;
    }
}

/**
 * Deletes the emitter sets of all handles, the current one last.
 * @param isKill Whether to kill the particles instead of fading them out.
 * @return Whether the current emitter set was alive.
 */
bool EffectEmitter::tryDeleteEmitter(bool isKill) {
    sead::ptcl::Handle* handle = mHandle;

    if (mHandles != nullptr) {
        for (s32 i = 0; i < mHandleNum; i++) {
            if (handle != mHandles[i]) {
                tryDeleteHandle(mHandles[i], isKill);
            }
        }
    }

    return tryDeleteHandle(handle, isKill);
}

/**
 * Kills or fades out the emitter set of a handle.
 * @param pHandle Handle of the emitter set.
 * @param isKill Whether to kill the particles instead of fading them out.
 * @return Whether the emitter set was alive.
 */
bool EffectEmitter::tryDeleteHandle(sead::ptcl::Handle* pHandle, bool isKill) {
    if (!pHandle->IsValid()) {
        return false;
    }

    nn::vfx::EmitterSet* emitterSet = pHandle->GetEmitterSet();
    if (!emitterSet->IsAlive()) {
        return false;
    }

    if (!emitterSet->IsCalcEnable() && !emitterSet->IsDrawEnable()) {
        emitterSet->SetCalcEnable(true);
        emitterSet->SetDrawEnable(true);
    }

    if (!isKill) {
        if (emitterSet->IsFadeRequest()) {
            return true;
        }

        f32 frame = mForceCalcFrame;
        for (nn::vfx::Emitter* emitter = emitterSet->GetAliveEmitter(0); emitter != nullptr;
             emitter = emitter->GetNextEmitter()) {
            if (emitter->GetFrame() > frame) {
                emitterSet->Fade();
                return true;
            }
        }
    }

    emitterSet->Kill(true);
    pHandle->Invalidate();
    return true;
}

/**
 * Stops or resumes calculation and drawing unless the emitter set is fading out.
 * @param isStop Whether to stop.
 */
void EffectEmitter::setStopCalcAndDraw(bool isStop) {
    nn::vfx::EmitterSet* emitterSet = mHandle->GetEmitterSet();

    if (emitterSet->IsFadeRequest()) {
        return;
    }

    emitterSet->SetCalcEnable(!isStop);
    emitterSet->SetDrawEnable(!isStop);
}

/**
 * Enables or disables drawing unless the emitter set is fading out.
 * @param isEnable Whether to draw.
 */
void EffectEmitter::setEnableDraw(bool isEnable) {
    nn::vfx::EmitterSet* emitterSet = mHandle->GetEmitterSet();

    if (emitterSet->IsFadeRequest()) {
        return;
    }

    emitterSet->SetDrawEnable(isEnable);
}

/**
 * Checks whether the current emitter set is alive.
 * @return Whether it is alive.
 */
bool EffectEmitter::isActive() const {
    if (mHandle->IsValid() && mHandle->GetEmitterSet()->IsAlive()) {
        return true;
    }

    return false;
}

/**
 * Checks whether a new emitter set may be created.
 * @return Whether emitting is allowed.
 */
bool EffectEmitter::isEnableEmit() const {
    const EmitterSetResourceInfo* info = mResourceInfo->mEmitterSetResourceInfo;

    if (info->mEmitterSetId < 0) {
        return false;
    }

    if (!info->mIsLoop && !info->mIsInfinity) {
        return true;
    }

    if (!isActive()) {
        return true;
    }

    return mHandle->GetEmitterSet()->IsFadeRequest();
}

/**
 * Checks whether the emitter was created this frame.
 * @return Whether this is the first frame.
 */
bool EffectEmitter::isFirstFrame() const {
    return mIsFirstFrame;
}

/**
 * Clears the first frame flag.
 */
void EffectEmitter::resetFirstFrame() {
    mIsFirstFrame = false;
}
}  // namespace al
