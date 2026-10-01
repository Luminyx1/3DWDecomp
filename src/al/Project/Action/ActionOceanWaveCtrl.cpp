#include "Project/Action/Common/ActionOceanWaveCtrl.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Project/OceanWave/OceanWaveKeeper.hpp"
#include "Project/OceanWave/OceanWaveUserInfo.hpp"

namespace al {
namespace {

const OceanWaveActionInfo* tryFindActionInfo(const OceanWaveUserInfo* pUserInfo,
                                             const char* pActionName) {
    if (pActionName == nullptr) {
        return nullptr;
    }

    if (pUserInfo->mActionInfoList == nullptr) {
        return nullptr;
    }

    return pUserInfo->mActionInfoList->tryFindInfo(pActionName);
}

}  // namespace

/**
 * Creates an ocean wave controller if the actor has ocean wave user info.
 * @param pActor Actor that owns the controller.
 * @return The new controller, or nullptr if the actor has no ocean wave user info.
 */
ActionOceanWaveCtrl* ActionOceanWaveCtrl::tryCreate(LiveActor* pActor) {
    if (pActor == nullptr) {
        return nullptr;
    }

    OceanWaveKeeper* keeper = pActor->mOceanWaveKeeper;

    if (keeper == nullptr) {
        return nullptr;
    }

    if (keeper->getUserInfo() == nullptr) {
        return nullptr;
    }

    return new ActionOceanWaveCtrl(pActor);
}

/**
 * Constructs the controller for an actor.
 * @param pActor Actor that owns the controller.
 */
ActionOceanWaveCtrl::ActionOceanWaveCtrl(LiveActor* pActor)
    : mParentActor(pActor), mOceanWaveKeeper(pActor->mOceanWaveKeeper) {}

void ActionOceanWaveCtrl::startAction(const char* pActionName) {
    const OceanWaveUserInfo* userInfo = mOceanWaveKeeper->getUserInfo();

    if (userInfo != nullptr) {
        mActionInfo = tryFindActionInfo(userInfo, pActionName);
    } else {
        mActionInfo = nullptr;
    }
}

void ActionOceanWaveCtrl::update(f32 frame, f32 frameRate) {
    if (mActionInfo == nullptr || mActionInfo->mPlayInfoList == nullptr) {
        return;
    }

    if (mOceanWaveKeeper->getDirector() == nullptr) {
        return;
    }

    s32 infoNum = mActionInfo->mPlayInfoList->getInfoNum();

    for (s32 i = 0; i < infoNum; i++) {
        const OceanWavePlayInfoInAction* info = mActionInfo->mPlayInfoList->getInfo(i);

        if (frameRate <= 0.0f) {
            continue;
        }

        if (info->mStartFrame <= frame && frame - frameRate < info->mStartFrame) {
            startOceanWave(mParentActor, info->mName);
        }
    }
}

}  // namespace al
