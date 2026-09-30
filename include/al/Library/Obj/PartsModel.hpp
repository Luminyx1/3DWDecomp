#pragma once

#include <math/seadMatrix.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

class PartsModel : public LiveActor {
public:
    PartsModel(const char* pName);

    void makeActorAppeared() override;
    void movementPaused(bool isPaused) override;
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
};

static_assert(sizeof(PartsModel) == 0x188);
}  // namespace al
