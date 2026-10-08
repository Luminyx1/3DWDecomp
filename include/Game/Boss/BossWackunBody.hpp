#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

class BossWackun;
class BossWackunHand;

/** @brief The moving cube body of BossWackun (only the parts the boss uses are reconstructed). */
class BossWackunBody : public al::LiveActor {
public:
    BossWackunBody(BossWackun* pBoss, BossWackunHand* pHand);

    void startDemo();
    void startBattle();
    void startMove(const sead::Vector3f& rAxis, s32 rotateType, s32 moveNum, s32 moveStep,
                   s32 turnStep);
    bool isDamage() const;
    bool isTurnEnd() const;
    void startRotate(s32 rotateType);
    void startLand(s32 rotateType);
    void startStandUp();
    void startRevival();
    void startDown(const sead::Vector3f& rDir);

    /** @return Local direction the attack sign shadow points along. */
    const sead::Vector3f& getSignLocalDir() const { return mSignLocalDir; }

    /** @return How many times the boss has been damaged. */
    s32 getDamageCount() const { return mDamageCount; }

private:
    u8 _144[0x54];
    sead::Vector3f mSignLocalDir;  // 0x198
    u8 _1A4[0x8];
    s32 mDamageCount;  // 0x1AC
    u8 _1B0[0x40];
};
static_assert(sizeof(BossWackunBody) == 0x1f0);
