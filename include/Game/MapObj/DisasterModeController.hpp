#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class FunctorBase;
class IUseSceneObjHolder;
}

class SuperBowser;
class DisasterBlockDirector;
class DisasterSpikeDirector;
class GameDataHolderAccessor;

/**
 * @brief Controls Bowser's Fury disaster mode (Fury Bowser's rampages).
 * @note Only the members used by reconstructed code are declared; the remaining state is kept
 * as padding.
 */
class DisasterModeController : public al::LiveActor, public al::ISceneObj {
  public:
    enum class State : int { Normal = 1, Disaster = 7 };
    class IUseEventReceiver {
    public:
        virtual void onDisasterModeStateChange(State) = 0;
    };
    void registerStateListener(IUseEventReceiver* listener) { if (!mStateListeners.isFull()) mStateListeners.pushBack(listener); }
    static DisasterModeController* tryGetController(const al::IUseSceneObjHolder* pUser);
    State getState();
    s32 getStateFrame();

    /**
     * @brief Get the step of the disaster flow.
     * @return The step of the disaster flow.
     */
    s32 getFlowStep() const { return mFlowStep; }
    static bool isLastBowserBattle(GameDataHolderAccessor accessor);

    DisasterBlockDirector* getBlockDirector() const { return mBlockDirector; }
    void endImmediate();
    void setSuperBowserV2(bool);
    void pause(bool);
    void resume(bool);
    void beginAndNeverEndDebug();
    void setGoalItemDisasterTrigger() { mGoalItemDisasterTrigger = true; }

    void forceDisasterForeshadowOff(bool isForce, s32 step);
    void setSkyEnable(bool isEnable);

    /**
     * @brief Set whether the disaster cycle may progress (cleared while a player is in a cloud
     * bonus area).
     * @param isEnable Whether the disaster cycle may progress.
     */
    void setDisasterProgressEnable(bool isEnable) { mIsDisasterProgressEnable = isEnable; }

    bool isSuperHardMode() const;
    bool isBowserHidden();
    bool isSuperBowserLeaving();
    void forceKillSuperBowserAttacks();
    void endInstantly(bool, bool);
    void tryJumpToRainWithFlash();
    bool isDisasterNerve();
    void forceKillSuperBowserLaser();
    bool setPostCutsceneDisasterFreezeTime();

    /**
     * @brief Set the callback run once the screen faded back in after ending disaster mode.
     * @param pFunctor The callback.
     */
    void setFadeInDoneFunctor(al::FunctorBase* pFunctor) { mFadeInDoneFunctor = pFunctor; }

    /**
     * @brief Access the Fury Bowser actor.
     * @return The Fury Bowser actor, or nullptr when absent.
     */
    SuperBowser* getSuperBowser() const { return mpSuperBowser; }

    /**
     * @brief Count the frames elapsed in disaster mode, including the saved offset.
     * @return The elapsed disaster-mode frames.
     */
    s32 calcDisasterFrames() const { return mDisasterFrames + mDisasterFramesOffset; }

    /**
     * @brief Check whether disaster mode is active.
     * @return True while disaster mode is active.
     */
    bool isRaining();
    al::LiveActor* getShell();
    bool isDisasterStarting();
    bool isInDisasterOrInstant() const;
    bool isWipeActive();

    /**
     * @brief Access the director of the disaster spikes.
     * @return The spike director.
     */
    DisasterSpikeDirector* getSpikeDirector() const { return mSpikeDirector; }
    bool isDisasterMode() const { return mIsDisasterMode; }

    /**
     * @brief Check whether objects should show their disaster mode animations.
     * @return Whether the disaster mode animations are active.
     */
    bool isDisasterModeAnim() const { return mIsDisasterModeAnim; }

    s32 calcProsperityStartAdditionalFramesMax();
    s32 calcFramesOfProsperity() const;
    void triggerAnticipationSwitch();
    void startRain();
    void clearTimeJumpFlags();

    /**
     * @brief Count the frames of peace elapsed before the next disaster.
     * @return The elapsed peace frames.
     */
    s32 getPeaceFrames() const { return mPeaceFrames; }

    /**
     * @brief Count the frames of rain that precede the disaster.
     * @return The pre-disaster rain frames.
     */
    s32 getPreRainFrames() const { return mPreRainFrames; }

    /**
     * @brief Get the disaster-mode frames elapsed (without the saved offset).
     * @return The elapsed disaster-mode frames.
     */
    s32 getDisasterFrames() const { return mDisasterFrames; }

    /**
     * @brief Jump the disaster timer to a frame.
     * @param frames The new elapsed disaster-mode frames.
     */
    void setDisasterFrames(s32 frames) {
        mDisasterFrames = frames;
        mDisasterFramesSync = frames;
    }

    /**
     * @brief Set the saved disaster-mode frame offset.
     * @param frames The new offset.
     */
    void setDisasterFramesOffset(s32 frames) { mDisasterFramesOffset = frames; }

    s32 getDisasterFramesOffset() const { return mDisasterFramesOffset; }

    /**
     * @brief Check whether the Black Sun only floats in place instead of rising.
     * @return True while the Black Sun floats.
     */
    bool isBlackSunFloating() const { return mIsBlackSunFloating; }

    /**
     * @brief Check whether the disaster timer is stopped.
     * @return True while the timer is stopped.
     */
    bool isTimeStopped() const { return mIsTimeStopped; }

    /**
     * @brief Check whether the disaster foreshadowing (the calm before Fury Bowser) is active.
     * @return True while the disaster is foreshadowed.
     */
    bool isDisasterForeshadow() const { return mIsDisasterForeshadow; }

    /**
     * @brief Access the second actor taking part in the disaster demos.
     * @return The actor, or nullptr when absent.
     */
    al::LiveActor* getDemoSubActor() const { return mDemoSubActor; }

  private:
    u8 mUnknown150[0x158 - 0x150];
    DisasterSpikeDirector* mSpikeDirector;  // 0x158
    DisasterBlockDirector* mBlockDirector;
    u8 mUnknown168[0x1ac - 0x168];
    s32 mPeaceFrames;  // 0x1ac
    u8 mUnknown1B0[0x1c0 - 0x1b0];
    s32 mDisasterFramesSync;  // 0x1c0
    s32 mDisasterFrames;
    u8 mUnknown1C8[0x20];
    SuperBowser* mpSuperBowser;
    al::LiveActor* mDemoSubActor;  // 0x1f0
    u8 mUnknown1F8;
    bool mIsDisasterMode;
    bool mIsDisasterModeAnim;  // 0x1fa
    bool mIsBlackSunFloating;  // 0x1fb
    u8 mUnknown1FC[0x204 - 0x1fc];
    bool mIsDisasterProgressEnable;  // 0x204
    u8 mUnknown205[0x23b - 0x205];
    bool mIsTimeStopped;  // 0x23b
    bool mGoalItemDisasterTrigger;  // 0x23c
    u8 mUnknown23D[0x25b - 0x23d];
    bool mIsDisasterForeshadow;  // 0x25b
    u8 mUnknown25C[0x29c - 0x25c];
    s32 mFlowStep;  // 0x29c
    u8 mUnknown2A0[0x2d0 - 0x2a0];
    s32 mDisasterFramesOffset;
    s32 mPreRainFrames;  // 0x2d4
    u8 mUnknown2D8[0x370 - 0x2d8];
    al::FunctorBase* mFadeInDoneFunctor;  // 0x370
    u8 mUnknown378[0x390 - 0x378];
    sead::PtrArray<IUseEventReceiver> mStateListeners;
};

using DisasterModeStateListener = DisasterModeController::IUseEventReceiver;
