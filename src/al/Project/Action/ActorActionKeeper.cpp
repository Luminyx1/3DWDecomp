#include "Project/Action/ActorActionKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Action/ActionAnimCtrl.hpp"
#include "Project/Action/ActionEffectCtrl.hpp"
#include "Project/Action/Common/ActionSeCtrl.hpp"
#include "Project/Action/Ctrl/ActionBgmCtrl.hpp"
#include "Project/Action/Ctrl/ActionFlagCtrl.hpp"
#include "Project/Action/Ctrl/ActionOceanWaveCtrl.hpp"
#include "Project/Action/Ctrl/ActionPadAndCameraCtrl.hpp"
#include "Project/Action/Ctrl/ActionScreenEffectCtrl.hpp"
#include "Project/Base/StringOpUtil.hpp"

namespace al {
    /**
     * @brief Creates the action keeper if the actor has any action resources.
     * @param pActor The actor.
     * @param pArchiveName The name of the actor's archive.
     * @param pActionListName The name of the actor's action list.
     * @return The created keeper, or nullptr if the actor has no action resources.
     */
    ActorActionKeeper* ActorActionKeeper::tryCreate(LiveActor* pActor, const char* pArchiveName, const char* pActionListName) {
        const char* archiveName = createStringIfInStack(getBaseName(pArchiveName));
        ActionAnimCtrl* animCtrl = ActionAnimCtrl::tryCreate(pActor, archiveName, pActionListName);
        NerveActionCtrl* nerveActionCtrl = pActor->getNerveKeeper() != nullptr ? pActor->getNerveKeeper()->mActionCtrl : nullptr;
        ActionFlagCtrl* flagCtrl = ActionFlagCtrl::tryCreate(pActor, pActionListName);
        ActionEffectCtrl* effectCtrl = ActionEffectCtrl::tryCreate(pActor);
        ActionSeCtrl* seCtrl = ActionSeCtrl::tryCreate(pActor->getAudioKeeper());
        ActionBgmCtrl* bgmCtrl = ActionBgmCtrl::tryCreate(pActor->getAudioKeeper());
        ActionPadAndCameraCtrl* padAndCameraCtrl = ActionPadAndCameraCtrl::tryCreate(pActor, getTransPtr(pActor), pActionListName);
        ActionScreenEffectCtrl* screenEffectCtrl = ActionScreenEffectCtrl::tryCreate(pActor);
        ActionOceanWaveCtrl* oceanWaveCtrl = ActionOceanWaveCtrl::tryCreate(pActor);

        if (animCtrl == nullptr && flagCtrl == nullptr && effectCtrl == nullptr && seCtrl == nullptr && bgmCtrl == nullptr &&
            padAndCameraCtrl == nullptr && screenEffectCtrl == nullptr && oceanWaveCtrl == nullptr) {
            return nullptr;
        }

        return new ActorActionKeeper(pActor, archiveName, animCtrl, nerveActionCtrl, flagCtrl, effectCtrl, seCtrl, bgmCtrl,
                                     oceanWaveCtrl, padAndCameraCtrl, screenEffectCtrl);
    }

    /**
     * @brief Starts an action.
     * @param pActionName The name of the action.
     */
    void ActorActionKeeper::startAction(const char* pActionName) {
        mIsActionRunning = true;
        if (mNerveActionCtrl == nullptr) {
            tryStartActionNoAnim(pActionName);
        }

        if (mAnimCtrl != nullptr) {
            mAnimCtrl->start(pActionName);
        }
    }

    /**
     * @brief Starts an action in every controller except the animation controller.
     * @param pActionName The name of the action.
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
     * @brief Starts the BGM of an action.
     * @param pActionName The name of the action.
     */
    void ActorActionKeeper::startBgmAction(const char* pActionName) {
        if (mBgmCtrl != nullptr) {
            mBgmCtrl->startAction(pActionName);
        }
    }

    /**
     * @brief Starts the effects of an action.
     * @param pActionName The name of the action.
     */
    void ActorActionKeeper::startEffectAction(const char* pActionName) {
        if (mEffectCtrl != nullptr) {
            mEffectCtrl->startAction(pActionName);
        }
    }

    /** @brief Updates the controllers before the actor's movement (does nothing). */
    void ActorActionKeeper::updatePrev() {}

    /** @brief Updates the controllers with the current action frame. */
    void ActorActionKeeper::updatePost() {
        if (mEffectCtrl != nullptr || mSeCtrl != nullptr || mBgmCtrl != nullptr || mPadAndCameraCtrl != nullptr ||
            mScreenEffectCtrl != nullptr || mOceanWaveCtrl != nullptr) {
            if (mNerveActionCtrl == nullptr || !isNewNerve(mActor)) {
                f32 frame = mNerveActionCtrl != nullptr ? static_cast<s32>(mActor->getNerveKeeper()->mNerveStep) - 1 :
                                                          getActionFrame(mActor);
                f32 frameRate = mNerveActionCtrl != nullptr ? 1.0f : getActionFrameRate(mActor);

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

        mIsActionRunning = false;
    }

    /** @brief Updates only the sound controller with the current action frame. */
    void ActorActionKeeper::updateSeActionCtrl() {
        if (mSeCtrl == nullptr) {
            return;
        }

        f32 frame = mNerveActionCtrl != nullptr ? static_cast<s32>(mActor->getNerveKeeper()->mNerveStep) - 1 :
                                                  getActionFrame(mActor);
        f32 frameRate = mNerveActionCtrl != nullptr ? 1.0f : getActionFrameRate(mActor);
        mSeCtrl->update(frame, frameRate);
    }

    /**
     * @brief Updates the sound and effect controllers for a frame jump.
     * @param prevFrame The previous action frame.
     * @param frame The new action frame.
     */
    void ActorActionKeeper::tryUpdateSeEffect(f32 prevFrame, f32 frame) {
        if (prevFrame == frame) {
            return;
        }

        if (mSeCtrl == nullptr && mEffectCtrl == nullptr) {
            return;
        }

        const char* actionName = getActionName(mActor);
        if (actionName == nullptr) {
            return;
        }

        f32 frameDelta;
        if (prevFrame > frame) {
            frameDelta = getActionFrameMax(mActor, actionName) - prevFrame + frame;
        } else {
            frameDelta = frame - prevFrame;
        }

        if (mSeCtrl != nullptr) {
            mSeCtrl->update(prevFrame, frameDelta);
        }

        if (mEffectCtrl != nullptr) {
            mEffectCtrl->update(prevFrame, frameDelta);
        }
    }

    /**
     * @brief Constructs the keeper.
     * @param pActor The actor.
     * @param pArchiveName The name of the actor's archive.
     * @param pAnimCtrl The animation controller.
     * @param pNerveActionCtrl The actor's nerve action controller.
     * @param pFlagCtrl The flag controller.
     * @param pEffectCtrl The effect controller.
     * @param pSeCtrl The sound controller.
     * @param pBgmCtrl The BGM controller.
     * @param pOceanWaveCtrl The ocean wave controller.
     * @param pPadAndCameraCtrl The pad rumble and camera controller.
     * @param pScreenEffectCtrl The screen effect controller.
     */
    ActorActionKeeper::ActorActionKeeper(LiveActor* pActor, const char* pArchiveName, ActionAnimCtrl* pAnimCtrl,
                                         NerveActionCtrl* pNerveActionCtrl, ActionFlagCtrl* pFlagCtrl,
                                         ActionEffectCtrl* pEffectCtrl, ActionSeCtrl* pSeCtrl, ActionBgmCtrl* pBgmCtrl,
                                         ActionOceanWaveCtrl* pOceanWaveCtrl, ActionPadAndCameraCtrl* pPadAndCameraCtrl,
                                         ActionScreenEffectCtrl* pScreenEffectCtrl)
        : mActor(pActor), mArchiveName(pArchiveName), mAnimCtrl(pAnimCtrl), mNerveActionCtrl(pNerveActionCtrl),
          mFlagCtrl(pFlagCtrl), mEffectCtrl(pEffectCtrl), mSeCtrl(pSeCtrl), mBgmCtrl(pBgmCtrl), mOceanWaveCtrl(pOceanWaveCtrl),
          mPadAndCameraCtrl(pPadAndCameraCtrl), mScreenEffectCtrl(pScreenEffectCtrl) {}

    /** @brief Finishes the initialization of the controllers. */
    void ActorActionKeeper::init() {
        if (mFlagCtrl != nullptr) {
            mFlagCtrl->initPost();
        }
    }
};
