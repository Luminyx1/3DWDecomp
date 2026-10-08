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
    explicit SuperBowser(const char* pName);
    void switchToV2(bool isV2);
    bool isRepelling();
    void pauseForDemo();
    void resumeFromDemo();
    void setLastPhase3Bowser(bool isLast);
    void killHealthBar();
    void activateCamera();
    void deactivateCamera();
    void requestDisappear(void (*pStartCallback)(void*), void (*pEndCallback)(void*), void* pArg,
                          bool isInstant);
    void end();
    void endByTime();
    void repel();
    bool isLeaving();
    void cancelFlow();
    void setUseSecondaryFlow(bool isUse);
    void enableLaserLightEffects(bool isEnable);

    /**
     * @brief Check whether Fury Bowser is out in the world.
     * @return The flag.
     */
    bool isActive() const { return mIsActive; }

    /** @brief Stop Fury Bowser's attack timers. */
    void stopAttackTimer() { mIsAttackTimerActive = false; }

    /** @brief Toggle Fury Bowser's laser attack. */
    void toggleLaserAttack() { mIsLaserAttack = !mIsLaserAttack; }

    /** @brief Remember that the first appearance cutscene was shown. */
    void setFirstAppearDone() { mIsFirstAppearDone = true; }

    /**
     * @brief Check whether Fury Bowser should leave as soon as possible.
     * @return The flag.
     */
    bool isLeaveRequested() const { return mIsLeaveRequested; }

    /**
     * @brief Set whether Fury Bowser should leave as soon as possible.
     * @param isRequested The flag.
     */
    void setLeaveRequested(bool isRequested) { mIsLeaveRequested = isRequested; }

    /**
     * @brief Set whether Fury Bowser may shoot fireballs.
     * @param isEnable The flag.
     */
    void setFireballEnable(bool isEnable) { mIsFireballEnable = isEnable; }

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
    u8 _144[0x150 - 0x144];
    bool mIsActive;  // 0x150
    u8 _151[0x1a8 - 0x151];
    bool mIsAttackTimerActive;  // 0x1a8
    u8 _1a9[0x1f0 - 0x1a9];
    bool mIsLaserAttack;  // 0x1f0
    bool mIsFirstAppearDone;  // 0x1f1
    bool mIsLeaveRequested;  // 0x1f2
    bool mIsFireballEnable;  // 0x1f3
    u8 _1f4[0x210 - 0x1f4];
    PlessieChaseState* mPlessieChaseState;  // 0x210
    u8 _218[0x50f - 0x218];
    bool mIsIgnoreBellOffsetY;  // 0x50f
};
