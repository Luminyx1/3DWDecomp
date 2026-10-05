#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class IUseSceneObjHolder;
}

class SuperBowser;

/**
 * @brief Controls Bowser's Fury disaster mode (Fury Bowser's rampages).
 * @note Only the members used by reconstructed code are declared; the remaining state is kept
 * as padding.
 */
class DisasterModeController : public al::LiveActor {
  public:
    static DisasterModeController* tryGetController(const al::IUseSceneObjHolder* pUser);

    void endImmediate();
    void setSuperBowserV2(bool);
    void pause(bool);
    void beginAndNeverEndDebug();
    void setGoalItemDisasterTrigger() { mGoalItemDisasterTrigger = true; }

    void forceDisasterForeshadowOff(bool isForce, s32 step);

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
    u8 mUnknown144[0x1c4 - 0x144]; // Unreconstructed state, from the actor tail padding.
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
};
