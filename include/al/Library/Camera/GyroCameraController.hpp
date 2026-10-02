#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace sead {
class LookAtCamera;
}

namespace al {
class AreaObjDirector;
class CollisionDirector;
class ControlAngleParam;
class IUseAudioKeeper;

/**
 * Rotation angles applied to the camera and the limits they are kept in.
 */
class GyroCameraControllerParam {
public:
    GyroCameraControllerParam();

    f32 mAngleV;
    f32 mAngleH;
    f32 mAngleVLimitMin;
    f32 mAngleVLimitMax;
    f32 mAngleHLimitMin;
    f32 mAngleHLimitMax;
};

static_assert(sizeof(GyroCameraControllerParam) == 0x18);

/**
 * Rotates a camera around its look-at position from the right stick (and, originally, the gyro),
 * keeping it inside the angle limits and moving it back when it ends up inside collision.
 */
class GyroCameraController : public IUseCollision, public IUseAreaObj {
public:
    GyroCameraController(IUseAudioKeeper* pAudioKeeper, CollisionDirector* pCollisionDirector,
                         AreaObjDirector* pAreaObjDirector, const bool* pIsReverseH,
                         const bool* pIsReverseV);

    void init(ControlAngleParam* pDefaultParam);
    void start();
    void calcGyroAngleValue(f32* pAngleH, f32* pAngleV);
    void update(sead::LookAtCamera* pCamera, ControlAngleParam* pParam, bool isUseParam);
    void setParam(sead::LookAtCamera* pCamera, ControlAngleParam* pParam, bool isUseParam);
    void updateRecover(const sead::LookAtCamera& rCamera, f32 baseAngleV, f32 baseAngleH);
    void updateGyroParams(const sead::LookAtCamera& rCamera, f32 baseAngleV, f32 baseAngleH,
                          bool isUseParam);
    void resetGyroParams();
    bool calcInputStick(sead::Vector2f* pOut, s32 port);

    CollisionDirector* getCollisionDirector() const override { return mCollisionDirector; }

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

private:
    inline void setAngleLimit(const ControlAngleParam& rParam);

    CollisionDirector* mCollisionDirector;
    AreaObjDirector* mAreaObjDirector;
    GyroCameraControllerParam mParam;
    f32 mOffsetAngleV = 0.0f;
    f32 mOffsetAngleH = 0.0f;
    ControlAngleParam* mDefaultParam = nullptr;
    bool mIsRecover = false;
    s32 mRecoverFrame = 0;
    f32 mRecoverStartAngleV = 0.0f;
    f32 mRecoverStartAngleH = 0.0f;
    f32 mTargetAngleV = 0.0f;
    f32 mTargetAngleH = 0.0f;
    f32 mGyroAngleH = 0.0f;
    f32 mGyroAngleV = 0.0f;
    IUseAudioKeeper* mAudioKeeper;
    const bool* mIsReverseH;
    const bool* mIsReverseV;
    f32 mPrevAngleV = 0.0f;
};

static_assert(sizeof(GyroCameraController) == 0x88);

}  // namespace al
