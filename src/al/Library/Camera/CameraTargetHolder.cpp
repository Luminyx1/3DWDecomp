#include "Library/Camera/CameraTargetHolder.hpp"

#include "Library/Camera/CameraSubTargetBase.hpp"
#include "Library/Camera/CameraTargetBase.hpp"
#include "Library/Play/Camera/ActorCameraTarget.hpp"
#include "Library/Player/PlayerUtil.hpp"

namespace al {

CameraTargetHolder::CameraTargetHolder(s32 maxTargets) : mViewTargetSize(maxTargets) {
    mViewTargetArray = new CameraTargetBase*[maxTargets];
    mViewTargetInfo = new ViewTargetInfo[maxTargets];

    for (s32 i = 0; i < mViewTargetSize; i++) {
        mViewTargetArray[i] = nullptr;
    }

    mTargetArray.allocBuffer(32, nullptr);
    mSubTargetArray.allocBuffer(32, nullptr);
    mPlacementSubTargetArray.allocBuffer(32, nullptr);
}

void CameraTargetHolder::initAfterPlacement(const PlayerHolder* pPlayerHolder) {
    mPlayerHolder = pPlayerHolder;
    _60 = false;

    if (mTargetArray.isEmpty()) {
        for (s32 i = 0; i < getPlayerNumMax(pPlayerHolder); i++) {
            if (mTargetArray.isFull()) {
                break;
            }

            mTargetArray.pushBack(
                new ActorCameraTarget(getPlayerActor(pPlayerHolder, i), 0.0f, nullptr));
        }
    }

    for (s32 i = 0; i < mViewTargetSize; i++) {
        ViewTargetInfo* info = &mViewTargetInfo[i];
        info->target = getViewTarget(i);
    }
}

CameraTargetBase* CameraTargetHolder::tryGetViewTarget(s32 index) const {
    CameraTargetBase* target = mViewTargetArray[index];

    if (target != nullptr) {
        return target;
    }

    if (mTargetArray.size() > 0) {
        return mTargetArray.front();
    }

    return nullptr;
}

void CameraTargetHolder::update() {
    for (s32 i = 0; i < mViewTargetSize; i++) {
        ViewTargetInfo* info = &mViewTargetInfo[i];
        CameraTargetBase* target = getViewTarget(i);
        info->hasTargetChanged = info->target != target;
        info->target = target;

        if (target != nullptr) {
            target->update();
        }
    }

    CameraSubTargetBase* topSubTarget = nullptr;

    if (!mSubTargetArray.isEmpty()) {
        topSubTarget = mSubTargetArray.front();
    } else if (!mPlacementSubTargetArray.isEmpty()) {
        topSubTarget = mPlacementSubTargetArray.front();
    }

    mTopSubTargetInfo.hasTargetChanged = mTopSubTargetInfo.target != topSubTarget;
    mTopSubTargetInfo.target = topSubTarget;

    if (topSubTarget != nullptr) {
        topSubTarget->update();
    }
}

s32 CameraTargetHolder::tryFindIndex(const CameraTargetBase* pTarget,
                                     const sead::PtrArray<CameraTargetBase>& rArray) {
    s32 index = 0;

    for (auto& target : rArray) {
        if (&target == pTarget) {
            return index;
        }

        index++;
    }

    return -1;
}

s32 CameraTargetHolder::tryFindIndex(const CameraSubTargetBase* pTarget,
                                     const sead::PtrArray<CameraSubTargetBase>& rArray) {
    s32 index = 0;

    for (auto& target : rArray) {
        if (&target == pTarget) {
            return index;
        }

        index++;
    }

    return -1;
}

bool CameraTargetHolder::tryRemovePtr(const CameraTargetBase* pTarget,
                                      sead::PtrArray<CameraTargetBase>& rArray) {
    s32 index = tryFindIndex(pTarget, rArray);

    if (index < 0) {
        return false;
    }

    rArray.erase(index);
    return true;
}

bool CameraTargetHolder::tryRemovePtr(const CameraSubTargetBase* pTarget,
                                      sead::PtrArray<CameraSubTargetBase>& rArray) {
    s32 index = tryFindIndex(pTarget, rArray);

    if (index < 0) {
        return false;
    }

    rArray.erase(index);
    return true;
}

void CameraTargetHolder::addTarget(CameraTargetBase* pTarget) {
    tryRemovePtr(pTarget, mTargetArray);
    pTarget->enableTarget();
    mTargetArray.pushFront(pTarget);
}

void CameraTargetHolder::removeTarget(CameraTargetBase* pTarget) {
    s32 index = tryFindIndex(pTarget, mTargetArray);

    if (index >= 0) {
        pTarget->disableTarget();
        mTargetArray.erase(index);
    }

    for (s32 i = 0; i < mViewTargetSize; i++) {
        if (mViewTargetArray[i] == pTarget) {
            pTarget->disableTarget();
            mViewTargetArray[i] = nullptr;
        }
    }
}

CameraTargetBase* CameraTargetHolder::getViewTarget(s32 index) const {
    return tryGetViewTarget(index);
}

bool CameraTargetHolder::isChangeViewTarget(s32 index) const {
    return mViewTargetInfo[index].hasTargetChanged;
}

CameraSubTargetBase* CameraTargetHolder::getTopSubTarget() const {
    return mTopSubTargetInfo.target;
}

void CameraTargetHolder::addSubTarget(CameraSubTargetBase* pTarget) {
    tryRemovePtr(pTarget, mSubTargetArray);
    pTarget->enableTarget();
    mSubTargetArray.pushFront(pTarget);
}

void CameraTargetHolder::removeSubTarget(CameraSubTargetBase* pTarget) {
    tryRemovePtr(pTarget, mSubTargetArray);
    pTarget->disableTarget();
}

void CameraTargetHolder::addPlacementSubTarget(CameraSubTargetBase* pTarget) {
    tryRemovePtr(pTarget, mPlacementSubTargetArray);
    pTarget->enableTarget();
    mPlacementSubTargetArray.pushFront(pTarget);
}

void CameraTargetHolder::removePlacementSubTarget(CameraSubTargetBase* pTarget) {
    tryRemovePtr(pTarget, mPlacementSubTargetArray);
    pTarget->disableTarget();
}

}  // namespace al
