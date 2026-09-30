#pragma once

#include <math/seadMatrix.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

class WarpedMtxPartsModel : public LiveActor {
public:
    WarpedMtxPartsModel(const char* pName);

    void makeActorAppeared() override;
    void calcAnim() override;
    void attackSensor(HitSensor* pSelf, HitSensor* pOther) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void initPartsMtx(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pArchiveName,
                      const sead::Matrix34f* pJointMtx, bool isUseFollowMtxScale);
    void initPartsSuffix(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pArchiveName,
                         const char* pSuffix, const sead::Matrix34f* pJointMtx,
                         bool isUseFollowMtxScale);
    void initPartsFixFile(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pArchiveName,
                          const char* pArchiveSuffix, const char* pSuffix);
    void updatePose();
    void syncHostVisible();

    LiveActor* mParentModel = nullptr;
    const sead::Matrix34f* mJointMtx = nullptr;
    bool mIsUseLocalPos = false;
    sead::Vector3f mLocalTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mLocalRotate = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mLocalScale = {1.0f, 1.0f, 1.0f};
    bool mIsHostHidden = false;
    bool mIsUseFollowMtxScale = false;
    bool mIsForceHide = false;
    bool mIsUseLocalScale = false;
    sead::Matrix34f mWarpedMtx = sead::Matrix34f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                                 0.0f, 0.0f, 1.0f, 0.0f);
};

static_assert(sizeof(WarpedMtxPartsModel) == 0x1b8);
}  // namespace al
