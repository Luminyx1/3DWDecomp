#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseSceneObjHolder;
}

class SuperBowser;

/**
 * @brief Controls Bowser's Fury disaster mode (Fury Bowser's rampages).
 * @note Only the members used by reconstructed code are declared; the actor base and the
 * remaining state are kept as padding.
 */
class DisasterModeController {
  public:
    static DisasterModeController* tryGetController(const al::IUseSceneObjHolder* pUser);

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
    bool isDisasterMode() const { return mIsDisasterMode; }

  private:
    u8 mUnknown0[0x1c4]; // Actor base and unreconstructed state.
    s32 mDisasterFrames;
    u8 mUnknown1C8[0x20];
    SuperBowser* mpSuperBowser;
    u8 mUnknown1F0[9];
    bool mIsDisasterMode;
    u8 mUnknown1FA[0xd6];
    s32 mDisasterFramesOffset;
};
