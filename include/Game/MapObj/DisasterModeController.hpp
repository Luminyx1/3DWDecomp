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
    static bool isLastBowserBattle(GameDataHolderAccessor accessor);

    DisasterBlockDirector* getBlockDirector() const { return mBlockDirector; }
    void endImmediate();
    void setSuperBowserV2(bool);
    void pause(bool);
    void resume(bool);
    void beginAndNeverEndDebug();
    void setGoalItemDisasterTrigger() { mGoalItemDisasterTrigger = true; }

    void forceDisasterForeshadowOff(bool isForce, s32 step);
    bool isSuperHardMode() const;
    bool isBowserHidden();
    bool isSuperBowserLeaving();
    void forceKillSuperBowserAttacks();
    void endInstantly(bool, bool);
    void tryJumpToRainWithFlash();

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
    u8 mUnknown168[0x1c4 - 0x168];
    s32 mDisasterFrames;
    u8 mUnknown1C8[0x20];
    SuperBowser* mpSuperBowser;
    al::LiveActor* mDemoSubActor;  // 0x1f0
    u8 mUnknown1F8;
    bool mIsDisasterMode;
    u8 mUnknown1FA[0x23c - 0x1fa];
    bool mGoalItemDisasterTrigger;  // 0x23c
    u8 mUnknown23D[0x25b - 0x23d];
    bool mIsDisasterForeshadow;  // 0x25b
    u8 mUnknown25C[0x2d0 - 0x25c];
    s32 mDisasterFramesOffset;
    u8 mUnknown2D4[0x370 - 0x2d4];
    al::FunctorBase* mFadeInDoneFunctor;  // 0x370
    u8 mUnknown378[0x390 - 0x378];
    sead::PtrArray<IUseEventReceiver> mStateListeners;
};

using DisasterModeStateListener = DisasterModeController::IUseEventReceiver;
