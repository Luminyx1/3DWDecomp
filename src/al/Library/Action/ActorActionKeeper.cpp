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
        pActor->getNerveKeeper() ? pActor->getNerveKeeper()->mActionCtrl : nullptr;
    ActionFlagCtrl* flagCtrl = ActionFlagCtrl::tryCreate(pActor, pSuffix);
    ActionEffectCtrl* effectCtrl = ActionEffectCtrl::tryCreate(pActor);
    ActionSeCtrl* seCtrl = ActionSeCtrl::tryCreate(pActor->getAudioKeeper());
    ActionBgmCtrl* bgmCtrl = ActionBgmCtrl::tryCreate(pActor->getAudioKeeper());
    ActionPadAndCameraCtrl* padAndCameraCtrl =
        ActionPadAndCameraCtrl::tryCreate(pActor, getTransPtr(pActor), pSuffix);
    ActionScreenEffectCtrl* screenEffectCtrl = ActionScreenEffectCtrl::tryCreate(pActor);
    ActionOceanWaveCtrl* oceanWaveCtrl = ActionOceanWaveCtrl::tryCreate(pActor);

    if (!animCtrl && !flagCtrl && !effectCtrl && !seCtrl && !bgmCtrl && !padAndCameraCtrl &&
        !screenEffectCtrl && !oceanWaveCtrl) {
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
    if (!mNerveActionCtrl) {
        tryStartActionNoAnim(pActionName);
    }
    return mAnimCtrl && mAnimCtrl->start(pActionName);
}

/**
 * Starts an action on all action controllers except the animation controller.
 * @param pActionName the name of the action
 */
void ActorActionKeeper::tryStartActionNoAnim(const char* pActionName) {
    if (mFlagCtrl) {
        mFlagCtrl->start(pActionName);
    }
    if (mEffectCtrl) {
        mEffectCtrl->startAction(pActionName);
    }
    if (mSeCtrl) {
        mSeCtrl->startAction(pActionName);
    }
    if (mBgmCtrl) {
        mBgmCtrl->startAction(pActionName);
    }
    if (mOceanWaveCtrl) {
        mOceanWaveCtrl->startAction(pActionName);
    }
    if (mPadAndCameraCtrl) {
        mPadAndCameraCtrl->startAction(pActionName);
    }
    if (mScreenEffectCtrl) {
        mScreenEffectCtrl->startAction(pActionName);
    }
}

/**
 * Starts a BGM action.
 * @param pActionName the name of the action
 */
void ActorActionKeeper::startBgmAction(const char* pActionName) {
    if (mBgmCtrl) {
        mBgmCtrl->startAction(pActionName);
    }
}

/**
 * Starts an effect action.
 * @param pActionName the name of the action
 */
void ActorActionKeeper::startEffectAction(const char* pActionName) {
    if (mEffectCtrl) {
        mEffectCtrl->startAction(pActionName);
    }
}

/**
 * Updates the keeper before the actor's movement.
 */
void ActorActionKeeper::updatePrev() {}

void ActorActionKeeper::updatePost() {
    if (mEffectCtrl || mSeCtrl || mBgmCtrl || mPadAndCameraCtrl || mScreenEffectCtrl ||
        mOceanWaveCtrl) {
        if (!mNerveActionCtrl || !isNewNerve(mActor)) {
            f32 frame = mNerveActionCtrl ? static_cast<s32>(mActor->getNerveKeeper()->mNerveStep) - 1 :
                                           getActionFrame(mActor);
            f32 frameRate = mNerveActionCtrl ? 1.0f : getActionFrameRate(mActor);
            if (mFlagCtrl) {
                mFlagCtrl->update(frame, frameRate);
            }
            if (mEffectCtrl) {
                mEffectCtrl->update(frame, frameRate);
            }
            if (mSeCtrl) {
                mSeCtrl->update(frame, frameRate);
            }
            if (mBgmCtrl) {
                mBgmCtrl->update(frame, frameRate);
            }
            if (mOceanWaveCtrl) {
                mOceanWaveCtrl->update(frame, frameRate);
            }
            if (mPadAndCameraCtrl) {
                mPadAndCameraCtrl->update(frame, frameRate);
            }
            if (mScreenEffectCtrl) {
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
    if (!mSeCtrl) {
        return;
    }
    f32 frame = mNerveActionCtrl ? static_cast<s32>(mActor->getNerveKeeper()->mNerveStep) - 1 :
                                   getActionFrame(mActor);
    f32 frameRate = mNerveActionCtrl ? 1.0f : getActionFrameRate(mActor);
    mSeCtrl->update(frame, frameRate);
}

void ActorActionKeeper::tryUpdateSeEffect(f32 frameFrom, f32 frameTo) {
    if (frameFrom == frameTo) {
        return;
    }
    if (!mSeCtrl && !mEffectCtrl) {
        return;
    }
    const char* actionName = getActionName(mActor);
    if (!actionName) {
        return;
    }
    f32 frameRate;
    if (frameFrom > frameTo) {
        frameRate = getActionFrameMax(mActor, actionName) - frameFrom + frameTo;
    } else {
        frameRate = frameTo - frameFrom;
    }
    if (mSeCtrl) {
        mSeCtrl->update(frameFrom, frameRate);
    }
    if (mEffectCtrl) {
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
    if (mFlagCtrl) {
        mFlagCtrl->initPost();
    }
}
}  // namespace al
