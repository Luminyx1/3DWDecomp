#include "Project/Action/Common/ActorActionKeeper.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Action/Common/ActionAnimCtrl.hpp"
#include "Project/Action/Common/ActionBgmCtrl.hpp"
#include "Project/Action/Common/ActionEffectCtrl.hpp"
#include "Project/Action/Common/ActionFlagCtrl.hpp"
#include "Project/Action/Common/ActionOceanWaveCtrl.hpp"
#include "Project/Action/Common/ActionPadAndCameraCtrl.hpp"
#include "Project/Action/Common/ActionScreenEffectCtrl.hpp"
#include "Project/Action/Common/ActionSeCtrl.hpp"
#include "Project/Base/StringOpUtil.hpp"

namespace al {
const char* createStringIfInStack(const char* pStr);
f32 getActionFrame(const LiveActor* pActor);
f32 getActionFrameRate(const LiveActor* pActor);
const char* getActionName(const LiveActor* pActor);

ActorActionKeeper* ActorActionKeeper::tryCreate(LiveActor* pActor, const char* pArchiveName,
                                                const char* pSuffix) {
    const char* name = createStringIfInStack(getBaseName(pArchiveName));
    ActionAnimCtrl* animCtrl = ActionAnimCtrl::tryCreate(pActor, name, pSuffix);
    NerveActionCtrl* nerveActionCtrl =
        (pActor->getNerveKeeper() != nullptr) ? pActor->getNerveKeeper()->mActionCtrl : nullptr;
    ActionFlagCtrl* flagCtrl = ActionFlagCtrl::tryCreate(pActor, pSuffix);
    ActionEffectCtrl* effectCtrl = ActionEffectCtrl::tryCreate(pActor);
    ActionSeCtrl* seCtrl = ActionSeCtrl::tryCreate(pActor->getAudioKeeper());
    ActionBgmCtrl* bgmCtrl = ActionBgmCtrl::tryCreate(pActor->getAudioKeeper());
    ActionPadAndCameraCtrl* padAndCameraCtrl =
        ActionPadAndCameraCtrl::tryCreate(pActor, getTransPtr(pActor), pSuffix);
    ActionScreenEffectCtrl* screenEffectCtrl = ActionScreenEffectCtrl::tryCreate(pActor);
    ActionOceanWaveCtrl* oceanWaveCtrl = ActionOceanWaveCtrl::tryCreate(pActor);

    if (animCtrl == nullptr && flagCtrl == nullptr && effectCtrl == nullptr && seCtrl == nullptr && bgmCtrl == nullptr && padAndCameraCtrl == nullptr &&
        (screenEffectCtrl == nullptr) && oceanWaveCtrl == nullptr) {
        return nullptr;
    }

    return new ActorActionKeeper(pActor, name, animCtrl, nerveActionCtrl, flagCtrl, effectCtrl,
                                 seCtrl, bgmCtrl, oceanWaveCtrl, padAndCameraCtrl,
                                 screenEffectCtrl);
}

/**
 * Starts an action on all action controllers.
 * @param pActionName the name of the action
 * @return true if the animation action was started
 */
bool ActorActionKeeper::startAction(const char* pActionName) {
    mIsActionStarted = true;

    if (mNerveActionCtrl == nullptr) {
        tryStartActionNoAnim(pActionName);
    }

    return (mAnimCtrl != nullptr) && mAnimCtrl->start(pActionName);
}

/**
 * Starts an action on all action controllers except the animation controller.
 * @param pActionName the name of the action
 */
void ActorActionKeeper::tryStartActionNoAnim(const char* pActionName) {
    if (mFlagCtrl != nullptr) {
        mFlagCtrl->start(pActionName);
    }

    if (mEffectCtrl != nullptr) {
        mEffectCtrl->startAction(pActionName);
    }

    if (mSeCtrl != nullptr) {
        mSeCtrl->startAction(pActionName);
    }

    if (mBgmCtrl != nullptr) {
        mBgmCtrl->startAction(pActionName);
    }

    if (mOceanWaveCtrl != nullptr) {
        mOceanWaveCtrl->startAction(pActionName);
    }

    if (mPadAndCameraCtrl != nullptr) {
        mPadAndCameraCtrl->startAction(pActionName);
    }

    if (mScreenEffectCtrl != nullptr) {
        mScreenEffectCtrl->startAction(pActionName);
    }
}

/**
 * Starts a BGM action.
 * @param pActionName the name of the action
 */
void ActorActionKeeper::startBgmAction(const char* pActionName) {
    if (mBgmCtrl != nullptr) {
        mBgmCtrl->startAction(pActionName);
    }
}

/**
 * Starts an effect action.
 * @param pActionName the name of the action
 */
void ActorActionKeeper::startEffectAction(const char* pActionName) {
    if (mEffectCtrl != nullptr) {
        mEffectCtrl->startAction(pActionName);
    }
}

/**
 * Updates the keeper before the actor's movement.
 */
void ActorActionKeeper::updatePrev() {}

void ActorActionKeeper::updatePost() {
    if (mEffectCtrl != nullptr || mSeCtrl != nullptr || mBgmCtrl != nullptr || mPadAndCameraCtrl != nullptr || mScreenEffectCtrl != nullptr ||
        (mOceanWaveCtrl != nullptr)) {
        if (mNerveActionCtrl == nullptr || !isNewNerve(mActor)) {
            f32 frame = (mNerveActionCtrl != nullptr) ? static_cast<s32>(mActor->getNerveKeeper()->mNerveStep) - 1 :
                                           getActionFrame(mActor);
            f32 frameRate = (mNerveActionCtrl != nullptr) ? 1.0f : getActionFrameRate(mActor);

            if (mFlagCtrl != nullptr) {
                mFlagCtrl->update(frame, frameRate);
            }

            if (mEffectCtrl != nullptr) {
                mEffectCtrl->update(frame, frameRate);
            }

            if (mSeCtrl != nullptr) {
                mSeCtrl->update(frame, frameRate);
            }

            if (mBgmCtrl != nullptr) {
                mBgmCtrl->update(frame, frameRate);
            }

            if (mOceanWaveCtrl != nullptr) {
                mOceanWaveCtrl->update(frame, frameRate);
            }

            if (mPadAndCameraCtrl != nullptr) {
                mPadAndCameraCtrl->update(frame, frameRate);
            }

            if (mScreenEffectCtrl != nullptr) {
                mScreenEffectCtrl->update(frame, frameRate);
            }
        }
    }

    mIsActionStarted = false;
}

/**
 * Updates the sound effect action controller.
 */
void ActorActionKeeper::updateSeActionCtrl() {
    if (mSeCtrl == nullptr) {
        return;
    }

    f32 frame = (mNerveActionCtrl != nullptr) ? static_cast<s32>(mActor->getNerveKeeper()->mNerveStep) - 1 :
                                   getActionFrame(mActor);
    f32 frameRate = (mNerveActionCtrl != nullptr) ? 1.0f : getActionFrameRate(mActor);
    mSeCtrl->update(frame, frameRate);
}

void ActorActionKeeper::tryUpdateSeEffect(f32 frameFrom, f32 frameTo) {
    if (frameFrom == frameTo) {
        return;
    }

    if (mSeCtrl == nullptr && mEffectCtrl == nullptr) {
        return;
    }

    const char* actionName = getActionName(mActor);

    if (actionName == nullptr) {
        return;
    }

    f32 frameRate;

    if (frameFrom > frameTo) {
        frameRate = getActionFrameMax(mActor, actionName) - frameFrom + frameTo;
    } else {
        frameRate = frameTo - frameFrom;
    }

    if (mSeCtrl != nullptr) {
        mSeCtrl->update(frameFrom, frameRate);
    }

    if (mEffectCtrl != nullptr) {
        mEffectCtrl->update(frameFrom, frameRate);
    }
}

ActorActionKeeper::ActorActionKeeper(LiveActor* pActor, const char* pActorName,
                                     ActionAnimCtrl* pAnimCtrl, NerveActionCtrl* pNerveActionCtrl,
                                     ActionFlagCtrl* pFlagCtrl, ActionEffectCtrl* pEffectCtrl,
                                     ActionSeCtrl* pSeCtrl, ActionBgmCtrl* pBgmCtrl,
                                     ActionOceanWaveCtrl* pOceanWaveCtrl,
                                     ActionPadAndCameraCtrl* pPadAndCameraCtrl,
                                     ActionScreenEffectCtrl* pScreenEffectCtrl)
    : mActor(pActor), mActorName(pActorName), mAnimCtrl(pAnimCtrl),
      mNerveActionCtrl(pNerveActionCtrl), mFlagCtrl(pFlagCtrl), mEffectCtrl(pEffectCtrl),
      mSeCtrl(pSeCtrl), mBgmCtrl(pBgmCtrl), mOceanWaveCtrl(pOceanWaveCtrl),
      mPadAndCameraCtrl(pPadAndCameraCtrl), mScreenEffectCtrl(pScreenEffectCtrl) {}

/**
 * Initializes the keeper after all actors are placed.
 */
void ActorActionKeeper::init() {
    if (mFlagCtrl != nullptr) {
        mFlagCtrl->initPost();
    }
}
}  // namespace al
