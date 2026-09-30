#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class EffectMtxSetter;
class PartsModel;
class RollingCubePoseKeeper;

class RollingCubeMapParts : public LiveActor {
public:
    RollingCubeMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void kill() override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void appearAndSetStart();
    void setNerveNextMovement(bool isNextFallKey);
    bool isNextFallKey() const;
    void exeWait();
    void exeStart();
    void exeRotate();
    s32 getMovementTime() const;
    void setNerveNextLand();
    void exeSlide();
    bool updateSlide();
    void exeFall();
    void setNerveNextFallLand();
    void exeLand();
    s32 getLandTime() const;
    void exeFallLand();
    void exeStop();
    bool isStop() const;

    RollingCubePoseKeeper* mRollingCubePoseKeeper = nullptr;
    sead::Matrix34f* mMoveLimitMtx = nullptr;
    PartsModel* mMoveLimitPartsModel = nullptr;
    sead::Matrix34f mLandEffectMtx = sead::Matrix34f::ident;
    EffectMtxSetter* mEffectMtxSetter = nullptr;
    sead::Quatf mInitialPoseQuat = sead::Quatf::unit;
    sead::Vector3f mInitialPoseTrans = sead::Vector3f::zero;
    sead::Quatf mCurrentPoseQuat = sead::Quatf::unit;
    sead::Vector3f mCurrentPoseTrans = sead::Vector3f::zero;
    sead::Vector3f mClippingTrans = sead::Vector3f::zero;
    s32 mMovementTime = 0;
    bool mIsStoppable = false;
};
}  // namespace al
