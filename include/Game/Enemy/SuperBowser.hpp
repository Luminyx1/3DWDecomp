#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

class SuperBowserLaserState;

/**
 * @brief Fury Bowser, the giant Bowser of Bowser's Fury.
 * @note Only the members used by reconstructed code are declared.
 */
class SuperBowser : public al::LiveActor {
  public:
    /// Placement Fury Bowser currently appears from.
    struct SpawnInfo {
        u8 _0[0xc];
        sead::Vector3f mFrontDir;  // 0xc
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
    void forceKillFireballs();
    bool isDoingEndingPreparations();
    s32 getLastPhase3CurrentIndex() const;
    bool isPlessieChaseBigRamp() const;
    SpawnInfo* getCurrentSpawnInfo();
    void setLaunchSpikeQueueCount(s32 count);
    void clearLaunchSpikeAmbientFrameCount();
    void tryLaunchSpike();
};
