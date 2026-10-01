#include "Project/Action/Common/ActionScreenEffectCtrl.hpp"

#include <math/seadMatrix.h>

#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/PostProcessing/RadialBlurDirector.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Yaml/YamlBridge.hpp"

namespace al {

/**
 * Constructs an entry with default radial blur settings.
 */
ActionScreenEffectCtrlInfo::ActionScreenEffectCtrlInfo() = default;

ActionScreenEffectCtrl::ActionScreenEffectCtrl(const LiveActor* pActor) : mParentActor(pActor) {
    ByamlIter iter(getModelOrAnimResourceYaml(pActor, "ScreenEffectCtrl", nullptr));
    mInfoCount = iter.getSize();
    mInfos = new ActionScreenEffectCtrlInfo[mInfoCount];

    for (s32 i = 0; i < mInfoCount; i++) {
        ActionScreenEffectCtrlInfo* info = &mInfos[i];
        ByamlIter infoIter;
        iter.tryGetIterByIndex(&infoIter, i);
        YamlWriterBridge bridge(&infoIter);
        info->serialize(bridge);
    }
}

/**
 * Creates a screen effect controller if the actor has a ScreenEffectCtrl resource.
 * @param pActor Actor that owns the controller.
 * @return The new controller, or nullptr if the resource does not exist.
 */
ActionScreenEffectCtrl* ActionScreenEffectCtrl::tryCreate(const LiveActor* pActor) {
    if (!isExistModelResource(pActor)) {
        return nullptr;
    }

    if (!isExistModelOrAnimResourceYaml(pActor, "ScreenEffectCtrl", nullptr)) {
        return nullptr;
    }

    return new ActionScreenEffectCtrl(pActor);
}

/**
 * Activates the screen effect entries that belong to an action.
 * @param pActionName Name of the started action.
 */
void ActionScreenEffectCtrl::startAction(const char* pActionName) {
    mActionName = pActionName;

    for (s32 i = 0; i < mInfoCount; i++) {
        mInfos[i].mIsActive = false;
    }

    for (s32 i = 0; i < mInfoCount; i++) {
        ActionScreenEffectCtrlInfo* info = &mInfos[i];

        if (isEqualString(mActionName, info->mActionName)) {
            info->mIsActive = true;
        }
    }
}

/**
 * Emits radial blurs for active entries whose start frame is passed this frame.
 * @param frame Current animation frame.
 * @param frameRate Current animation frame rate.
 */
void ActionScreenEffectCtrl::update(f32 frame, f32 frameRate) {
    for (s32 i = 0; i < mInfoCount; i++) {
        ActionScreenEffectCtrlInfo* info = &mInfos[i];

        if (!info->mIsActive || !info->mRadialBlur.mIsEnable) {
            continue;
        }

        if (!alAnimFunction::checkPass(frame, frameRate,
                                       static_cast<f32>(info->mRadialBlur.mStartFrame))) {
            continue;
        }

        s32 blurFrame = info->mRadialBlur.mBlurFrame;
        const char* jointName = info->mRadialBlur.mJointName;
        sead::Vector3f pos;

        if (jointName != nullptr && jointName[0] != '\0') {
            pos.setMul(*getJointMtxPtr(mParentActor, jointName), info->mRadialBlur.mPosOffset);
        } else {
            pos.setAdd(getTrans(mParentActor), info->mRadialBlur.mPosOffset);
        }

        emitRadialBlur(mParentActor, pos, info->mRadialBlur.mRadiusBegin,
                       info->mRadialBlur.mRadiusEnd, blurFrame, -1);
    }
}

}  // namespace al
