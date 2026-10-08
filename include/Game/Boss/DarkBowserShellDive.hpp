#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class HitSensor;
}  // namespace al

class DarkBowser;

/**
 * @brief Fury Bowser's shell dive attack state.
 * @note Only what reconstructed code needs is declared so far.
 */
class DarkBowserShellDive : public al::NerveStateBase {
public:
    DarkBowserShellDive(DarkBowser* pHost, const al::ActorInitInfo& rInfo);

    void setLevel(s32 level);
    bool isAerial() const;
    bool isEnding() const;
    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther);
    void forceRecover();

    /**
     * @brief Makes the next dive start from a knock back (after a side kick or a bomb hit).
     * @param recoverFrame Frames to stay knocked back before diving.
     */
    void requestKnockBack(s32 recoverFrame) {
        mIsKnockBack = true;
        mKnockBackFrame = recoverFrame;
    }

    /** @brief Enables the extra (low health) dive pattern. */
    void enableExtraPattern() { mPatternFlags |= 1; }

    /**
     * @brief Whether the dive wants the aerial camera.
     * @return True while airborne.
     */
    bool isRequestAerialCamera() const { return mIsRequestAerialCamera; }

private:
    u8 _11[0x30 - 0x11];  // starts in the tail padding of al::NerveStateBase
    s32 mKnockBackFrame;              // 0x30
    bool mIsKnockBack;                // 0x34
    u8 _35[0x1DC - 0x35];
    s32 mPatternFlags;                // 0x1DC
    u8 _1E0[0x1F1 - 0x1E0];
    bool mIsRequestAerialCamera;      // 0x1F1
    u8 _1F2[0x200 - 0x1F2];
};
static_assert(sizeof(DarkBowserShellDive) == 0x200);
