#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

#include "MapObj/BindPuppeteer.hpp"

namespace al {
class ActorInitInfo;
class HitSensor;
}  // namespace al

class GoalPole;

/**
 * @brief Drives one player bound to the goal pole: catch, slide down, jump off and pose.
 */
class GoalPoleBindPuppeteer : public BindPuppeteer {
public:
    GoalPoleBindPuppeteer(const char* pName, GoalPole* pPole, const al::ActorInitInfo& rInfo,
                          const sead::Matrix34f* pPoleMtx);

    void startBind(al::HitSensor* pPlayerSensor, al::HitSensor* pBinderSensor, s32 catchNum);
    void update();
    void onCatchFailure();
    bool isCatchSuccess() const;
    bool isCatchJust() const;
    bool isEndCatchMove() const;
    bool isEnableStartFall() const;
    bool isEndFalling() const;
    void startFall(s32 heightOrder, s32 catchNum, const GoalPoleBindPuppeteer* pUnder);
    bool isEnableStartJump() const;
    void startJump();
    bool isEnableAppearFlag() const;
    f32 calcCatchHeightRate() const;
    static s32 getJumpStartDelayFrame();
    static int getFallFrameMax();

    /**
     * @brief Height on the pole at which the player caught it.
     * @return The catch height.
     */
    f32 getCatchHeight() const { return mCatchHeight; }

    /**
     * @brief Order in which the player caught the pole (0 for the first).
     * @return The catch index.
     */
    s32 getCatchIndex() const { return mCatchIndex; }

    /**
     * @brief Rank of the catch height among all players (0 for the highest).
     * @return The height order.
     */
    s32 getHeightOrder() const { return mHeightOrder; }

private:
    u8 mUnreconstructed1c[0x64];  // starts in the tail padding of BindPuppeteer
    f32 mCatchHeight;  // 0x80
    u8 mUnreconstructed84[4];
    s32 mCatchIndex;  // 0x88
    s32 mHeightOrder;  // 0x8c
    u8 mUnreconstructed90[0x40];
};

static_assert(sizeof(GoalPoleBindPuppeteer) == 0xd0);
