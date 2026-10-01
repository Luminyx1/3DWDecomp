#include "Project/Action/Common/ActionPadAndCameraCtrl.hpp"

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Controller/PadRumbleFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Controller/PadRumbleKeeper.hpp"

namespace al {
namespace {

bool isOverDistance(f32 distance, f32 limit) {
    if (limit < 0.0f) {
        return false;
    }

    return distance > limit;
}

void startCameraShakeImpl(const LiveActor* pActor, const char* pShakeName,
                          const char* pActionName) {
    if (pActor->mActorSceneInfo->isSingleMode) {
        if (pShakeName != nullptr) {
            startCameraShakeByAction(pActor, pShakeName, pActionName, -1, 0);
        }
    } else if (pShakeName != nullptr) {
        requestStartCameraShake(pActor, pShakeName);
    }
}

void startPadRumbleImpl(const LiveActor* pActor, const char* pRumbleName, s32 port) {
    if (pRumbleName != nullptr) {
        alPadRumbleFunction::startPadRumble(pActor, pRumbleName, port, false);
    }
}

}  // namespace

/**
 * Constructs zero-initialized pad and camera data.
 */
ActionPadAndCameraData::ActionPadAndCameraData() = default;

/**
 * Constructs an entry with default frames and distances.
 */
ActionPadAndCameraCtrlInfo::ActionPadAndCameraCtrlInfo() = default;

/**
 * Creates a pad and camera controller if the actor has a PadAndCameraCtrl resource.
 * @param pActor Actor that owns the controller.
 * @param pPos Position used for distance checks.
 * @param pSuffix Suffix of the resource file name.
 * @return The new controller, or nullptr if the resource does not exist.
 */
ActionPadAndCameraCtrl* ActionPadAndCameraCtrl::tryCreate(const LiveActor* pActor,
                                                          const sead::Vector3f* pPos,
                                                          const char* pSuffix) {
    if (!isExistModelResource(pActor)) {
        return nullptr;
    }

    StringTmp<256> fileName;

    if (!tryGetActorInitFileName(&fileName, pActor, "PadAndCameraCtrl", pSuffix) &&
        !tryGetActorAnimInitFileName(&fileName, pActor, "PadAndCameraCtrl", pSuffix)) {
        createFileNameBySuffix(&fileName, "PadAndCameraCtrl", pSuffix);
    }

    if (!isExistModelOrAnimResourceYaml(pActor, fileName.cstr(), nullptr)) {
        return nullptr;
    }

    return new ActionPadAndCameraCtrl(pActor, pPos, pSuffix);
}

/**
 * Creates a pad and camera controller from a resource if it has a PadAndCameraCtrl file.
 * @param pActor Actor that owns the controller.
 * @param pPos Position used for distance checks.
 * @param pResource Resource that holds the file.
 * @return The new controller, or nullptr if the file does not exist.
 */
ActionPadAndCameraCtrl* ActionPadAndCameraCtrl::tryCreate(const LiveActor* pActor,
                                                          const sead::Vector3f* pPos,
                                                          Resource* pResource) {
    StringTmp<256> fileName;

    if (!tryGetActorInitFileIterAndName(nullptr, &fileName, pResource, "PadAndCameraCtrl",
                                        nullptr)) {
        createFileNameBySuffix(&fileName, "PadAndCameraCtrl", nullptr);
    }

    if (!isExistResourceYaml(pResource, fileName.cstr(), nullptr)) {
        return nullptr;
    }

    return new ActionPadAndCameraCtrl(pActor, pPos, pResource, fileName.cstr());
}

/**
 * Activates the entries of an action and applies those that start at frame 0.
 * @param pActionName Name of the started action.
 */
void ActionPadAndCameraCtrl::startAction(const char* pActionName) {
    mActionName = pActionName;

    for (s32 i = 0; i < mInfoCount; i++) {
        ActionPadAndCameraCtrlInfo* info = &mInfos[i];
        info->mIsActive = false;
        info->mLastUpdateFrame = -1.0f;
    }

    for (s32 i = 0; i < mInfoCount; i++) {
        ActionPadAndCameraCtrlInfo* info = &mInfos[i];

        if (!isEqualString(mActionName, info->mActionName)) {
            continue;
        }

        info->mIsActive = true;

        if (info->mStartFrame <= 0) {
            updatePadAndCamera(info);
            info->mLastUpdateFrame = 0.0f;
        }
    }
}

void ActionPadAndCameraCtrl::updatePadAndCamera(const ActionPadAndCameraCtrlInfo* pInfo) {
    if (pInfo->mIsUseDemo) {
        startCameraShakeImpl(mParentActor, pInfo->mCameraShakeName, pInfo->mActionName);

        if (pInfo->mIsUsePadRumbleKeeper) {
            if (pInfo->mPadRumbleName != nullptr) {
                alPadRumbleFunction::startPadRumble(mParentActor, pInfo->mPadRumbleName,
                                                    mPadRumbleKeeper->getPort(), false);
            }

            return;
        }

        s32 playerNum = getPlayerNumMaxComplete(mParentActor);

        for (s32 i = 0; i < playerNum; i++) {
            s32 port = getPlayerPort(mParentActor, i);

            if (port < 1) {
                continue;
            }

            if (isPlayerDead(mParentActor, i) && !pInfo->mIsUseDeadPlayer) {
                continue;
            }

            if (pInfo->mPadRumbleName != nullptr) {
                alPadRumbleFunction::startPadRumble(mParentActor, pInfo->mPadRumbleName, port,
                                                    false);
            }
        }

        return;
    }

    f32 distance;

    if (tryFindNearestPlayerDisatanceFromTarget(&distance, mParentActor, *mPos) &&
        !isOverDistance(distance, pInfo->mDistanceInvalid)) {
        if (isOverDistance(distance, pInfo->mDistanceFar)) {
            startCameraShakeImpl(mParentActor, pInfo->mCameraShakeNameFar, pInfo->mActionName);
        } else if (isOverDistance(distance, pInfo->mDistanceNear)) {
            startCameraShakeImpl(mParentActor, pInfo->mCameraShakeNameMiddle, pInfo->mActionName);
        } else {
            startCameraShakeImpl(mParentActor, pInfo->mCameraShakeName, pInfo->mActionName);
        }
    }

    s32 playerNum = getPlayerNumMaxComplete(mParentActor);

    for (s32 i = 0; i < playerNum; i++) {
        if (isPlayerDead(mParentActor, i) &&
            (getPlayerPort(mParentActor, i) < 1 || !pInfo->mIsUseDeadPlayer)) {
            continue;
        }

        s32 port = getPlayerPort(mParentActor, i);

        if (pInfo->mIsUsePadRumbleKeeper && port != mPadRumbleKeeper->getPort()) {
            continue;
        }

        f32 playerDistance = (getPlayerPos(mParentActor, i) - *mPos).length();

        if (isOverDistance(playerDistance, pInfo->mDistanceInvalid)) {
            continue;
        }

        if (isOverDistance(playerDistance, pInfo->mDistanceFar)) {
            startPadRumbleImpl(mParentActor, pInfo->mPadRumbleNameFar, port);
        } else if (isOverDistance(playerDistance, pInfo->mDistanceNear)) {
            startPadRumbleImpl(mParentActor, pInfo->mPadRumbleNameMiddle, port);
        } else {
            startPadRumbleImpl(mParentActor, pInfo->mPadRumbleName, port);
        }
    }
}

/**
 * Applies pad rumbles and camera shakes of active entries within their frame ranges.
 * @param frame Current animation frame.
 * @param frameRate Current animation frame rate.
 */
void ActionPadAndCameraCtrl::update(f32 frame, f32 frameRate) {
    for (s32 i = 0; i < mInfoCount; i++) {
        ActionPadAndCameraCtrlInfo* info = &mInfos[i];

        if (!info->mIsActive) {
            continue;
        }

        if (alAnimFunction::checkPass(frame, frameRate, static_cast<f32>(info->mStartFrame))) {
            info->mIsPlaying = true;
        } else if (!info->mIsPlaying) {
            continue;
        }

        if (info->mLastUpdateFrame < frame) {
            updatePadAndCamera(info);
        }

        if (frameRate <= 0.0f || info->mEndFrame < 0 ||
            alAnimFunction::checkPass(frame, frameRate, static_cast<f32>(info->mEndFrame))) {
            info->mIsPlaying = false;
        }
    }
}

/**
 * Constructs the controller from the actor's PadAndCameraCtrl file.
 * @param pActor Actor that owns the controller.
 * @param pPos Position used for distance checks.
 * @param pSuffix Suffix of the resource file name.
 */
ActionPadAndCameraCtrl::ActionPadAndCameraCtrl(const LiveActor* pActor, const sead::Vector3f* pPos,
                                               const char* pSuffix)
    : mParentActor(pActor), mPos(pPos) {
    StringTmp<128> fileName;

    if (!tryGetActorInitFileName(&fileName, pActor, "PadAndCameraCtrl", pSuffix) &&
        !tryGetActorAnimInitFileName(&fileName, pActor, "PadAndCameraCtrl", pSuffix)) {
        createFileNameBySuffix(&fileName, "PadAndCameraCtrl", pSuffix);
    }

    init(getModelOrAnimResourceYaml(pActor, fileName.cstr(), nullptr), false);
}

/**
 * Reads all entries from the yaml data.
 * @param pYaml Byaml data of the PadAndCameraCtrl file.
 * @param isUnused Unused flag.
 */
void ActionPadAndCameraCtrl::init(const u8* pYaml, bool isUnused) {
    ByamlIter iter(pYaml);
    mInfoCount = iter.getSize();
    mInfos = new ActionPadAndCameraCtrlInfo[mInfoCount];

    for (s32 i = 0; i < mInfoCount; i++) {
        ActionPadAndCameraCtrlInfo* info = &mInfos[i];
        ByamlIter infoIter;
        iter.tryGetIterByIndex(&infoIter, i);
        infoIter.tryGetStringByKey(&info->mActionName, "ActionName");
        infoIter.tryGetIntByKey(&info->mStartFrame, "StartFrame");
        infoIter.tryGetIntByKey(&info->mEndFrame, "EndFrame");
        infoIter.tryGetBoolByKey(&info->mIsUseDemo, "IsUseDemo");
        infoIter.tryGetBoolByKey(&info->mIsUsePadRumbleKeeper, "IsUsePadRumbleKeeper");
        infoIter.tryGetBoolByKey(&info->mIsUseDeadPlayer, "IsUseDeadPlayer");
        infoIter.tryGetStringByKey(&info->mPadRumbleName, "PadRumbleName");
        infoIter.tryGetStringByKey(&info->mCameraShakeName, "CameraShakeName");
        infoIter.tryGetFloatByKey(&info->mDistanceNear, "DistanceNear");
        infoIter.tryGetFloatByKey(&info->mDistanceFar, "DistanceFar");
        infoIter.tryGetFloatByKey(&info->mDistanceInvalid, "DistanceInvalid");
        infoIter.tryGetStringByKey(&info->mPadRumbleNameMiddle, "PadRumbleNameMiddle");
        infoIter.tryGetStringByKey(&info->mPadRumbleNameFar, "PadRumbleNameFar");
        infoIter.tryGetStringByKey(&info->mCameraShakeNameMiddle, "CameraShakeNameMiddle");
        infoIter.tryGetStringByKey(&info->mCameraShakeNameFar, "CameraShakeNameFar");
        infoIter.tryGetIntByKey(&info->mCameraViewTarget, "CameraViewTarget");
    }
}

/**
 * Constructs the controller from a file of a resource.
 * @param pActor Actor that owns the controller.
 * @param pPos Position used for distance checks.
 * @param pResource Resource that holds the file.
 * @param pFileName Name of the file.
 */
ActionPadAndCameraCtrl::ActionPadAndCameraCtrl(const LiveActor* pActor, const sead::Vector3f* pPos,
                                               Resource* pResource, const char* pFileName)
    : mParentActor(pActor), mPos(pPos) {
    init(findResourceYaml(pResource, pFileName, nullptr), false);
}

}  // namespace al
