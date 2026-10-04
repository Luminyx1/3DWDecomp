#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "MapObj/BindPuppeteer.hpp"

namespace al {
class ActorInitInfo;
class HitSensor;
class LiveActor;
}  // namespace al

class BindWarpEffect;
class Dokan;
class DummyCameraTarget;

/**
 * @brief Moves a bound player into a warp pipe, warps it to the destination pipe and moves it
 * out again.
 */
class DokanBindPuppeteer : public BindPuppeteer {
public:
    DokanBindPuppeteer(const char* pName, bool isSide, bool isWorldWarp, al::LiveActor* pHost);

    void endBind(const PlayerBindEndParam* pParam) override;
    void cancelBind() override;

    void init(const al::ActorInitInfo& rInfo);
    void startBind(al::HitSensor* pPlayerSensor, al::HitSensor* pBinderSensor,
                   const al::LiveActor* pDokan, const Dokan* pDestDokan, bool isBindAll,
                   bool isHipDrop, bool isRolling);
    void setupHipDropIn(const al::LiveActor* pDokan);
    void startBindWorldWarp(al::HitSensor* pPlayerSensor, al::HitSensor* pBinderSensor,
                            const al::LiveActor* pDokan, bool isBindAll, bool isHipDrop);
    void update();
    void setOnPlayerCountMax();
    bool isDeactive() const;
    bool isEnableStartBind(bool isHipDrop, bool isBindAll) const;
    bool isWaitStartWarp() const;
    bool isWaitStartWorldWarp() const;
    void warp(s32 index, s32 num, bool isUseCamera);
    void dokanOut();
    const char* getDokanOutActionName() const;
    void updateHipDropPos();

    void exeDeactive();
    void exeDokanInMove();
    void exeDokanInDown();
    void exeDokanInSideMove();
    void exeDokanInHipDrop();
    void exeDokanInRolling();
    void exeWaitStartWarp();
    void exeForceBindWarp();
    void exeForceBindWarpEnd();
    void exeWaitStartDokanOut();
    void exeDokanOutUp();
    void exeWaitStartWorldWarp();

private:
    void startCameraTarget();
    void endCameraTarget();

    s32 mOnPlayerCount = 0;                       // 0x1c
    sead::Matrix34f mStartMtx;                    // 0x20
    sead::Matrix34f mEndMtx;                      // 0x50
    const al::LiveActor* mDokan = nullptr;        // 0x80
    const Dokan* mDestDokan = nullptr;            // 0x88
    sead::Vector3f mOutOffset;                    // 0x90
    bool mIsInWater = false;                      // 0x9c
    bool mIsSide;                                 // 0x9d
    bool mIsWorldWarp;                            // 0x9e
    BindWarpEffect* mWarpEffect = nullptr;        // 0xa0
    bool mIsHipDrop = false;                      // 0xa8
    bool mIsRolling = false;                      // 0xa9
    DummyCameraTarget* mCameraTarget = nullptr;   // 0xb0
    al::LiveActor* mHost;                         // 0xb8
    bool mIsSingleMode = false;                   // 0xc0
    bool mIsCameraTargetOn = false;               // 0xc1
    sead::Vector3f mVelocity;                     // 0xc4
    f32 mSpeedH = 0.0f;                           // 0xd0
    al::HitSensor* mBinderSensor = nullptr;       // 0xd8
};

static_assert(sizeof(DokanBindPuppeteer) == 0xe0);
