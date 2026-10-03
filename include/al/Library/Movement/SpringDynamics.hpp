#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {

/**
 * Point mass pulled towards a target by a spring once it is farther away than a rest length.
 */
class SpringDynamics {
public:
    SpringDynamics(f32 damping);
    void calcForce(const sead::Vector3f& rTarget, f32 length, f32 springRate, bool isForce);
    void updatePos();

private:
    sead::Vector3f mPos;
    sead::Vector3f mVelocity;
    f32 mDamping;
};

static_assert(sizeof(SpringDynamics) == 0x1C);

/**
 * Point mass hanging from a target with a maximum distance.
 */
class HangDynamics {
public:
    HangDynamics(f32 damping);
    void limitDistance(const sead::Vector3f& rTarget, f32 maxDistance);
    void calcForce(const sead::Vector3f& rTarget, f32 length, f32 springRate);
    void updatePos();

private:
    sead::Vector3f mPos;
    sead::Vector3f mVelocity;
    f32 mDamping;
};

static_assert(sizeof(HangDynamics) == 0x1C);

/**
 * Rigid rod rotating around its root position.
 */
class RotateHangDynamics {
public:
    RotateHangDynamics();
    void initPosLength(const sead::Vector3f& rRootPos, f32 length, const sead::Vector3f& rDir);
    void moveRootPosAndFix(const sead::Vector3f& rRootPos);
    void moveRootPos(const sead::Vector3f& rRootPos);
    void applyForceTip(const sead::Vector3f& rForce);
    void applyForcePos(const sead::Vector3f& rForce, const sead::Vector3f& rPos);
    void applyGravity(const sead::Vector3f& rGravity);
    void smoothOmega(f32 rate, const sead::Vector3f& rTargetOmega);
    void update(f32 deltaTime);

    const sead::Quatf& getQuat() const { return mQuat; }

    const sead::Vector3f& getOmega() const { return mOmega; }

    f32 getLength() const { return mLength; }

    f32 getLimitAngle() const { return mLimitAngle; }

    const sead::Vector3f& getRootPos() const { return mRootPos; }

    const sead::Vector3f& getTipPos() const { return mTipPos; }

    /**
     * Calculates the arm from the root to the tip.
     * @return The vector from the root position to the tip position.
     */
    sead::Vector3f calcArm() const { return mTipPos - mRootPos; }

private:
    sead::Quatf mQuat;
    sead::Quatf mInvQuat;
    sead::Vector3f mOmega;
    sead::Vector3f mTorque;
    f32 mLength;
    f32 mMass;
    f32 mDamping;
    f32 mRootMoveRate;
    f32 mGravityRate;
    f32 mLimitAngle;
    f32 mLimitBounceRate;
    f32 mSmoothRate;
    sead::Vector3f mRootPos;
    sead::Vector3f mTipPos;
};

static_assert(sizeof(RotateHangDynamics) == 0x70);

/**
 * Chain of rotating rods, each one hanging from the tip of the previous one.
 */
class RotateHangDynamicsArray {
public:
    RotateHangDynamicsArray(s32 num);
    void initPosQuatAll(const sead::Vector3f& rRootPos, const sead::Vector3f& rDir);
    void updateDynamics(f32 deltaTime, const sead::Vector3f& rGravity,
                        const sead::Vector3f& rRootPos);
    void calcTipVelocity(sead::Vector3f* pVelocity) const;
    void calcTipAcc(sead::Vector3f* pAcc) const;
    f32 calcRotateRate() const;
    void applyForceTipExp(const sead::Vector3f& rForce);
    void applyForcePosAll(const sead::Vector3f& rForce, const sead::Vector3f& rPos, s32 index);
    void calcPosQuatFromRootDist(sead::Vector3f* pPos, sead::Quatf* pQuat, s32* pIndex,
                                 f32 dist) const;

private:
    sead::PtrArray<RotateHangDynamics> mArray;
    sead::Vector3f mPrevTipPos;
    sead::Vector3f mTipVelocity;
};

static_assert(sizeof(RotateHangDynamicsArray) == 0x28);

/**
 * Chain of spheres connected by penalty forces.
 */
class PenaltyHangDynamics {
public:
    /**
     * Rigid sphere of the chain.
     */
    class Parts {
    public:
        f32 calcMass() const;
        void applyForce(const sead::Vector3f& rPos, const sead::Vector3f& rForce);
        void update(f32 deltaTime);
        void resetState(const sead::Vector3f& rPos, const sead::Vector3f& rVelocity);

        sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
        sead::Vector3f mVelocity = {0.0f, 0.0f, 0.0f};
        sead::Vector3f mForce = {0.0f, 0.0f, 0.0f};
        sead::Quatf mQuat = {1.0f, 0.0f, 0.0f, 0.0f};
        sead::Quatf mInvQuat = {1.0f, 0.0f, 0.0f, 0.0f};
        sead::Vector3f mWorldOmega = {0.0f, 0.0f, 0.0f};
        sead::Vector3f mLocalOmega = {0.0f, 0.0f, 0.0f};
        sead::Vector3f mTorque = {0.0f, 0.0f, 0.0f};
        f32 mRadius = 100.0f;
        f32 _6c = 0.01f;
        f32 _70 = 0.99f;
        f32 mAngularDamping = 0.95f;
        bool _78 = false;
    };

    PenaltyHangDynamics(s32 num, const sead::Vector3f& rUnused);
    void initTrans(const sead::Vector3f& rTrans);
    void applyGravityAllParts(const sead::Vector3f& rGravity);
    void collideSphere(const sead::Vector3f& rCenter, const sead::Vector3f& rVelocity,
                       f32 radius, f32 rate);
    void update(f32 deltaTime, const sead::Vector3f& rRootPos);

private:
    sead::PtrArray<Parts> mParts;
};

static_assert(sizeof(PenaltyHangDynamics::Parts) == 0x7C);

}  // namespace al
