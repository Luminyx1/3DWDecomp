#pragma once

#include <math/seadVector.h>

#include "Library/HostIO/IUseHioNode.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace al {

class CameraTargetCollideInfoHolder : public IUseCollision, public IUseHioNode {
public:
    CameraTargetCollideInfoHolder();

    void update(bool isValid, const sead::Vector3f& rTrans, const sead::Vector3f& rUp,
                const sead::Vector3f& rGravity);
    void reset();
    bool isExistUnderWall() const;
    bool tryCalcSlopeDownFrontDirH(sead::Vector3f* pDir) const;

    CollisionDirector* getCollisionDirector() const override { return mCollisionDirector; }

    void setCollisionDirector(CollisionDirector* pDirector) { mCollisionDirector = pDirector; }

    bool isExistCollisionUnderTarget() const { return mIsExistCollisionUnderTarget; }

    const sead::Vector3f& getTargetCollisionPos() const { return mTargetCollisionPos; }

    const sead::Vector3f& getTargetCollisionNormal() const { return mTargetCollisionNormal; }

    bool isExistSlopeCollisionUnderTarget() const { return mIsExistSlopeCollisionUnderTarget; }

    f32 getSlopeCollisionUpSpeed() const { return mSlopeCollisionUpSpeed; }

    f32 getSlopeCollisionDownSpeed() const { return mSlopeCollisionDownSpeed; }

    void set2D(bool is2D) { mIs2D = is2D; }

private:
    CollisionDirector* mCollisionDirector = nullptr;
    bool mIs2D = false;
    bool mIsExistCollisionUnderTarget = false;
    sead::Vector3f mGravity = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTargetCollisionNormal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTargetCollisionPos = {0.0f, 0.0f, 0.0f};
    bool mIsExistSlopeCollisionUnderTarget = false;
    sead::Vector3f mSlopeDownDir = {0.0f, 0.0f, 0.0f};
    f32 mSlopeCollisionDownSpeed = 0.0f;
    f32 mSlopeCollisionUpSpeed = 0.0f;
    s32 mInvalidCount = 0;
};

static_assert(sizeof(CameraTargetCollideInfoHolder) == 0x58);

}  // namespace al
