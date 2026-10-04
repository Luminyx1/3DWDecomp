#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
class AudioDirector;
class CameraInfo;
class CameraTicket;
class CollisionObj;
}  // namespace al

class ActorStateGiantBlow;
class BindPuppeteerGroup;
class DokanBindPuppeteer;
class DokanGuideBalloon;

/**
 * @brief A warp pipe. Players enter it and are carried by puppeteers to the linked destination
 * pipe (or out of this one again).
 */
class Dokan : public al::LiveActor {
public:
    Dokan(const char* pName);

    void init(const al::ActorInitInfo& rInfo) override;
    void initAfterPlacement() override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther,
                    al::HitSensor* pSelf) override;
    void control() override;
    void startFarLod() override;
    void endFarLod() override;
    void startClipped() override;
    void endClipped() override;

    void setIsHideModel(bool isHide);
    void appearBySwitch();
    void appearBySwitchInstant();
    void activate();
    bool isTypeOutOnly() const;
    bool startGiantBlowKill(const al::SensorMsg* pMsg, al::HitSensor* pOther);
    bool isEnableEnter();
    void setLayout(const al::HitSensor* pPlayerSensor);
    DokanBindPuppeteer* getPuppeteer(const al::HitSensor* pPlayerSensor) const;
    void endWait();
    void appearOutOnly();
    void disappearOutOnly();
    bool isWait() const;
    void updatePuppeteer();
    bool isWarpStart() const;
    void tryWarp();
    bool isFinishWaitOutCamera();

    void exeWait();
    void exeAppear();
    void exeDisappear();
    void exePlayerIn();
    void exePlayerOutWithCameraWait();
    void exePlayerOutWithCameraTicketWait();
    void exePlayerOutWaitForDstAnimDone();
    void exePlayerOut();
    void exePlayerOutWaitInput();
    void exeWaitAppearDestDokan();
    void exeDeactive();
    void exeGiantBlow();

private:
    Dokan* mPairDokan = nullptr;                          // 0x148
    s32 mType = 0;                                        // 0x150
    al::PlacementId* mPlacementId = new al::PlacementId;  // 0x158
    BindPuppeteerGroup* mPuppeteerGroup = nullptr;        // 0x160
    BindPuppeteerGroup* mBindOrderGroup = nullptr;        // 0x168
    bool mIsHideModel = false;                            // 0x170
    bool mIsRequestedBindAll = false;                     // 0x171
    bool mIsUpsideDown = false;                           // 0x172
    bool mIsSide = false;                                 // 0x173
    bool mIsGold = false;                                 // 0x174
    bool mIsCameraInterpolate = false;                    // 0x175
    bool mIsControlPlayerOut = false;                     // 0x176
    bool mIsInWater = false;                              // 0x177
    bool mIsBreakGiantMario = true;                       // 0x178
    s32 mOutCameraPlayStep = 0;                           // 0x17c
    al::CameraInfo* mCameraInfo = nullptr;                // 0x180
    al::CameraTicket* mCameraTicket = nullptr;            // 0x188
    ActorStateGiantBlow* mGiantBlowState = nullptr;       // 0x190
    al::LiveActor* mBreakModel = nullptr;                 // 0x198
    al::LiveActor* mTraceModel = nullptr;                 // 0x1a0
    DokanGuideBalloon** mGuideBalloons = nullptr;         // 0x1a8
    al::CollisionObj* mCollisionObj = nullptr;            // 0x1b0
    al::AudioDirector* mAudioDirector = nullptr;          // 0x1b8
    bool mIsSetBgmVolume = false;                         // 0x1c0
    bool mIsSingleMode = false;                           // 0x1c1
    bool mIsHipDropEnter = false;                         // 0x1c2
    al::HitSensor* mBindSensor = nullptr;                 // 0x1c8
    al::HitSensor* mInvinciblePlayerSensor = nullptr;     // 0x1d0
    al::LiveActor* mBindPlayer = nullptr;                 // 0x1d8
    s32 mSaveFlagId = -1;                                 // 0x1e0
    sead::Vector3f mPlayerOutPos;                         // 0x1e4
    bool mIsWaitInputLanding;                             // 0x1f0
    bool mIsPlayerOutPosSet;                              // 0x1f1
    f32 mPipeInitOffset = 0.0f;                           // 0x1f4
    f32 mPipeInitOffsetDecayRate = 0.0f;                  // 0x1f8
    bool mIsIgnorePlayerControlAfterExit = false;         // 0x1fc
};

static_assert(sizeof(Dokan) == 0x200);
