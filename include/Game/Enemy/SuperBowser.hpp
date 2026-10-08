#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

class SuperBowserGiantFireballState;
class SuperBowserLaserState;

/**
 * @brief Fury Bowser, the giant Bowser of Bowser's Fury.
 * @note Only the members used by reconstructed code are declared.
 */
class SuperBowser : public al::LiveActor {
  public:
    /// Placement Fury Bowser currently appears from.
    struct SpawnInfo {
        sead::Vector3f mTrans;     // 0x0
        sead::Vector3f mFrontDir;  // 0xc
    };

    /// State of the Plessie chase of the last battle.
    struct PlessieChaseState {
        u8 _0[0x40];
        s32 mTier;  // 0x40
    };

    bool isLastPhase3Bowser();
    void disappear(bool isInstant);
    void tryDamageDarkBowser();
    void setFacingDirection(sead::Vector3f& rDir);
    void startBowserExitCamera(bool, bool, bool);
    void endBowserExitCamera(bool isSkip);
    bool isHidden();
    f32 getJumpRate();
    SuperBowserLaserState* getLaserState();
    SuperBowserGiantFireballState* getGiantFireballState();
    s32 getGiantFireballShootCount();
    void forceKillFireballs();
    bool isDoingEndingPreparations();
    s32 getLastPhase3CurrentIndex() const;
    bool isPlessieChaseBigRamp() const;
    SpawnInfo* getCurrentSpawnInfo();
    void notifyPlayerHit();
    void setLaunchSpikeQueueCount(s32 count);
    void clearLaunchSpikeAmbientFrameCount();
    void tryLaunchSpike();
    bool hasLanded();
    void doPlessieChaseHit(bool isReset);
    void resetPlessieChase(s32 tier);
    bool canPlessieChaseHit();
    bool isPlessieChaseTierJump() const;

    /**
     * @brief Check whether the Plessie chase is in its first tier.
     * @return True when a chase is running and it is in its first tier.
     */
    bool isPlessieChaseFirstTier() const {
        return mPlessieChaseState != nullptr && mPlessieChaseState->mTier == 0;
    }

    /**
     * @brief Check whether the Giga Bells ignore their pattern's vertical offset.
     * @return The flag.
     */
    bool isIgnoreBellOffsetY() const { return mIsIgnoreBellOffsetY; }

private:
    u8 _148[0x210 - 0x148];
    PlessieChaseState* mPlessieChaseState;  // 0x210
    u8 _218[0x50f - 0x218];
    bool mIsIgnoreBellOffsetY;  // 0x50f
};
