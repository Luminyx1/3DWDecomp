#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

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
    bool isDoingEndingPreparations();
    s32 getLastPhase3CurrentIndex() const;
    bool isPlessieChaseBigRamp() const;
    SpawnInfo* getCurrentSpawnInfo();
    void setLaunchSpikeQueueCount(s32 count);
    void clearLaunchSpikeAmbientFrameCount();
    void tryLaunchSpike();
};
