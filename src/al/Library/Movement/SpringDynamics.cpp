#include "Library/Movement/SpringDynamics.hpp"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuatCalcCommon.h>

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs the spring dynamics at the origin with no velocity.
 * @param damping Factor the velocity is multiplied with every update.
 */
SpringDynamics::SpringDynamics(f32 damping)
    : mPos(sead::Vector3f::zero), mVelocity(sead::Vector3f::zero), mDamping(damping) {}

/**
 * Accelerates the point towards the rest position around the target.
 * @param rTarget Position the spring is attached to.
 * @param length Rest length of the spring.
 * @param springRate Strength of the spring.
 * @param isForce Whether to apply the force even when the point is closer than the rest length.
 */
void SpringDynamics::calcForce(const sead::Vector3f& rTarget, f32 length, f32 springRate,
                               bool isForce) {
    sead::Vector3f diff = mPos;
    diff -= rTarget;
    f32 distance;
    sead::Vector3f dir;
    separateScalarAndDirection(&distance, &dir, diff);

    if (distance < length && !isForce) {
        return;
    }

    dir *= length;
    mVelocity += (dir + rTarget - mPos) * springRate;
    mVelocity *= mDamping;
}

/**
 * Moves the point by its velocity.
 */
void SpringDynamics::updatePos() {
    mPos += mVelocity;
}

/**
 * Constructs the hang dynamics at the origin with no velocity.
 * @param damping Factor the velocity is multiplied with every update.
 */
HangDynamics::HangDynamics(f32 damping)
    : mPos(sead::Vector3f::zero), mVelocity(sead::Vector3f::zero), mDamping(damping) {}

/**
 * Pulls the point back so it is not farther than a given distance from the target.
 * @param rTarget Position the point hangs from.
 * @param maxDistance Maximum distance from the target.
 */
void HangDynamics::limitDistance(const sead::Vector3f& rTarget, f32 maxDistance) {
    sead::Vector3f diff = mPos - rTarget;
    f32 distance;
    sead::Vector3f dir;
    separateScalarAndDirection(&distance, &dir, diff);

    if (distance > maxDistance) {
        mPos = dir * maxDistance + rTarget;
    }
}

/**
 * Accelerates the point so its next position does not exceed the given length from the target.
 * @param rTarget Position the point hangs from.
 * @param length Length of the rope.
 * @param springRate Strength of the pull back.
 */
void HangDynamics::calcForce(const sead::Vector3f& rTarget, f32 length, f32 springRate) {
    sead::Vector3f nextPos = mPos + mVelocity;
    sead::Vector3f diff = nextPos;
    diff -= rTarget;
    f32 distance;
    sead::Vector3f dir;
    separateScalarAndDirection(&distance, &dir, diff);

    if (distance > length) {
        f32 diffLength = diff.length();

        if (diffLength > 0.0f) {
            diff *= length / diffLength;
        }
    }

    sead::Vector3f force = (rTarget + diff - nextPos) * springRate;
    f32 forceLength;
    sead::Vector3f forceDir;
    separateScalarAndDirection(&forceLength, &forceDir, force);
    mVelocity += force;
    mVelocity *= mDamping;
}

/**
 * Moves the point by its velocity.
 */
void HangDynamics::updatePos() {
    mPos += mVelocity;
}

/**
 * Constructs a rod of length 100 hanging straight down from the origin.
 */
RotateHangDynamics::RotateHangDynamics()
    : mQuat(1.0f, 0.0f, 0.0f, 0.0f), mInvQuat(1.0f, 0.0f, 0.0f, 0.0f) {
    mLength = 100.0f;
    mMass = 1.0f;
    mTorque.set(0.0f, 0.0f, 0.0f);
    mDamping = 0.98f;
    mRootMoveRate = 5.0f;
    mGravityRate = 10.0f;
    mLimitAngle = 360.0f;
    mLimitBounceRate = 1.0f;
    mSmoothRate = 0.0f;
    mRootPos.set(0.0f, 0.0f, 0.0f);
    mTipPos.set(0.0f, -100.0f, 0.0f);
    mQuat.set(sead::Quatf::unit);
    mOmega.set(sead::Vector3f::zero);
}

/**
 * Places the rod at a root position pointing away from a direction.
 * @param rRootPos Root position of the rod.
 * @param length Length of the rod.
 * @param rDir Direction from the tip to the root.
 */
void RotateHangDynamics::initPosLength(const sead::Vector3f& rRootPos, f32 length,
                                       const sead::Vector3f& rDir) {
    mRootPos.set(rRootPos);
    mLength = length;
    mTipPos = rRootPos - rDir * length;
    mOmega.set(sead::Vector3f::zero);
    makeQuatUpNoSupport(&mQuat, rDir);
}

/**
 * Moves the root position and lets the rod hang straight down from it.
 * @param rRootPos New root position.
 */
void RotateHangDynamics::moveRootPosAndFix(const sead::Vector3f& rRootPos) {
    mRootPos.set(rRootPos);
    sead::Vector3f up = sead::Vector3f::ey;
    mTipPos = mRootPos - up * mLength;
}

/**
 * Moves the root position and applies the resulting torque.
 * @param rRootPos New root position.
 */
void RotateHangDynamics::moveRootPos(const sead::Vector3f& rRootPos) {
    sead::Vector3f force = (mTipPos - rRootPos) * mRootMoveRate;
    mTorque += calcArm().cross(force);
    mRootPos.set(rRootPos);
}

/**
 * Applies a force at the tip of the rod.
 * @param rForce Force to apply.
 */
void RotateHangDynamics::applyForceTip(const sead::Vector3f& rForce) {
    mTorque += calcArm().cross(rForce);
}

/**
 * Applies a force at a position.
 * @param rForce Force to apply.
 * @param rPos Position the force is applied at.
 */
void RotateHangDynamics::applyForcePos(const sead::Vector3f& rForce, const sead::Vector3f& rPos) {
    mTorque += (rPos - mRootPos).cross(rForce);
}

/**
 * Applies gravity at the tip of the rod.
 * @param rGravity Gravity direction.
 */
void RotateHangDynamics::applyGravity(const sead::Vector3f& rGravity) {
    sead::Vector3f force = mGravityRate * rGravity;
    mTorque += calcArm().cross(force);
}

/**
 * Moves the angular velocity towards a target angular velocity.
 * @param rate Interpolation rate scale.
 * @param rTargetOmega Target angular velocity.
 */
void RotateHangDynamics::smoothOmega(f32 rate, const sead::Vector3f& rTargetOmega) {
    mOmega += (rTargetOmega - mOmega) * mSmoothRate * rate;
}

/**
 * Integrates the accumulated torque and updates the tip position.
 * @param deltaTime Time step.
 */
void RotateHangDynamics::update(f32 deltaTime) {
    sead::Vector3f localTorque;
    localTorque.setRotated(mInvQuat, mTorque);
    f32 invInertia = 1.0f / (0.4f * mMass * mLength * mLength);
    sead::Vector3f localAcc = localTorque * invInertia * deltaTime;
    localAcc.rotate(mQuat);
    mOmega = (mOmega + localAcc) * mDamping;
    sead::QuatCalcCommon<f32>::applyAngularVelocity(mQuat, mOmega, deltaTime);

    sead::Vector3f up;
    calcQuatUp(&up, mQuat);

    f32 angle = calcAngleDegree(sead::Vector3f::ey, up);

    if (mLimitAngle < angle) {
        sead::Vector3f front;
        calcQuatFront(&front, mQuat);
        sead::Vector3f axis;
        axis.setCross(sead::Vector3f::ey, up);
        normalizeOrZero(&axis);

        if (mLimitBounceRate > 0.0f) {
            sead::Quatf limitQuat;
            limitQuat.setAxisAngle(axis, mLimitAngle);
            sead::Vector3f limitUp;
            limitUp.setRotated(limitQuat, sead::Vector3f::ey);
            makeQuatUpFront(&mQuat, limitUp, front);
            f32 dot = axis.dot(mOmega);
            mOmega -= axis * (dot + mLimitBounceRate * dot);
        } else {
            mOmega += axis * (mLimitBounceRate * axis.dot(mOmega));
        }
    }

    mInvQuat.setInverse(mQuat);
    calcQuatUp(&up, mQuat);
    mTipPos = mRootPos - up * mLength;
    mTorque.set(sead::Vector3f::zero);
}

/**
 * Constructs a chain of rods.
 * @param num Number of rods.
 */
RotateHangDynamicsArray::RotateHangDynamicsArray(s32 num) {
    mArray.allocBuffer(num, nullptr);

    for (s32 i = 0; i < num; i++) {
        mArray.pushBack(new RotateHangDynamics());
    }
}

/**
 * Places all rods of the chain in a straight line.
 * @param rRootPos Root position of the chain.
 * @param rDir Direction from the tip to the root.
 */
void RotateHangDynamicsArray::initPosQuatAll(const sead::Vector3f& rRootPos,
                                             const sead::Vector3f& rDir) {
    sead::Vector3f pos = rRootPos;
    s32 num = mArray.size();

    for (s32 i = 0; i < num; i++) {
        RotateHangDynamics* dynamics = mArray[i];
        dynamics->initPosLength(pos, dynamics->getLength(), rDir);
        pos -= dynamics->getLength() * rDir;
    }

    mPrevTipPos.set(mArray.back()->getTipPos());
    mTipVelocity.set(0.0f, 0.0f, 0.0f);
}

/**
 * Updates all rods of the chain.
 * @param deltaTime Time step.
 * @param rGravity Gravity direction.
 * @param rRootPos Root position of the chain.
 */
void RotateHangDynamicsArray::updateDynamics(f32 deltaTime, const sead::Vector3f& rGravity,
                                             const sead::Vector3f& rRootPos) {
    mTipVelocity.setSub(mArray(mArray.size() - 1)->getTipPos(), mPrevTipPos);
    mPrevTipPos.set(mArray.back()->getTipPos());
    const sead::Vector3f* rootPos = &rRootPos;

    for (s32 i = 0; i < mArray.size(); i++) {
        sead::Vector3f newRootPos = *rootPos;
        RotateHangDynamics* dynamics = mArray[i];
        dynamics->applyGravity(rGravity);
        dynamics->moveRootPos(newRootPos);

        if (dynamics != mArray.back()) {
            RotateHangDynamics* next = mArray(i + 1);
            sead::Vector3f omega = (dynamics->getOmega() + next->getOmega()) * 0.5f;
            dynamics->smoothOmega(deltaTime, omega);
            next->smoothOmega(deltaTime, omega);
        }

        dynamics->update(deltaTime);
        rootPos = &dynamics->getTipPos();
    }
}

/**
 * Calculates the velocity of the tip of the chain.
 * @param pVelocity Output velocity.
 */
void RotateHangDynamicsArray::calcTipVelocity(sead::Vector3f* pVelocity) const {
    pVelocity->setSub(mArray(mArray.size() - 1)->getTipPos(), mPrevTipPos);
}

/**
 * Calculates the acceleration of the tip of the chain.
 * @param pAcc Output acceleration.
 */
void RotateHangDynamicsArray::calcTipAcc(sead::Vector3f* pAcc) const {
    pAcc->setSub(mArray(mArray.size() - 1)->getTipPos() - mPrevTipPos, mTipVelocity);
}

/**
 * Calculates how far the chain is rotated relative to the limit angle.
 * @return Rotation rate between 0 and 1.
 */
f32 RotateHangDynamicsArray::calcRotateRate() const {
    sead::Vector3f rootPos = mArray.front()->getRootPos();
    sead::Vector3f tipPos = mArray.back()->getTipPos();
    sead::Vector3f dir = rootPos - tipPos;
    normalizeOrZero(&dir);
    f32 rate = calcAngleDegree(sead::Vector3f::ey, dir) /
               mArray(mArray.size() - 1)->getLimitAngle();
    return sead::Mathf::clamp(rate, 0.0f, 1.0f);
}

/**
 * Applies a force at the tip of every rod, scaled linearly from the root to the tip.
 * @param rForce Force applied at the tip of the chain.
 */
void RotateHangDynamicsArray::applyForceTipExp(const sead::Vector3f& rForce) {
    s32 num = mArray.size();

    for (s32 i = num - 1; i >= 0; i--) {
        f32 rate = static_cast<f32>(i) / static_cast<f32>(num - 1);
        mArray(i)->applyForceTip(rate * rForce);
    }
}

/**
 * Applies a force at a position to every rod, scaled linearly from the root to the tip.
 * @param rForce Force to apply.
 * @param rPos Position the force is applied at.
 * @param index Last rod the force is applied to, or -1 for all.
 */
void RotateHangDynamicsArray::applyForcePosAll(const sead::Vector3f& rForce,
                                               const sead::Vector3f& rPos, s32 index) {
    s32 num = mArray.size();

    for (s32 i = num - 1; i >= 0; i--) {
        if (index != -1 && i > index) {
            continue;
        }

        f32 rate = static_cast<f32>(i) / static_cast<f32>(num - 1);
        mArray(i)->applyForcePos(rate * rForce, rPos);
    }
}

/**
 * Calculates the position and rotation at a distance along the chain.
 * @param pPos Output position, may be null.
 * @param pQuat Output rotation, may be null.
 * @param pIndex Output index of the rod, may be null.
 * @param dist Distance from the root of the chain.
 */
void RotateHangDynamicsArray::calcPosQuatFromRootDist(sead::Vector3f* pPos, sead::Quatf* pQuat,
                                                      s32* pIndex, f32 dist) const {
    if (dist < 0.0f) {
        if (pPos != nullptr) {
            pPos->set(mArray.front()->getRootPos());
        }

        if (pQuat != nullptr) {
            *pQuat = mArray(0)->getQuat();
        }

        return;
    }

    for (s32 i = 0; i < mArray.size(); i++) {
        RotateHangDynamics* dynamics = mArray[i];

        if (dist <= dynamics->getLength()) {
            if (pPos != nullptr) {
                lerpVec(pPos, dynamics->getRootPos(), dynamics->getTipPos(),
                        dist / dynamics->getLength());
            }

            if (pQuat != nullptr) {
                *pQuat = dynamics->getQuat();
            }

            if (pIndex != nullptr) {
                *pIndex = i;
            }

            return;
        }

        dist -= dynamics->getLength();
    }

    if (pPos != nullptr) {
        pPos->set(mArray.back()->getTipPos());
    }

    if (pQuat != nullptr) {
        *pQuat = mArray(mArray.size() - 1)->getQuat();
    }
}

/**
 * Calculates the mass of the sphere.
 * @return Mass of the sphere.
 */
f32 PenaltyHangDynamics::Parts::calcMass() const {
    return 4.0f / 3.0f * sead::Mathf::pi() * mRadius * mRadius * mRadius;
}

/**
 * Applies a force at a position.
 * @param rPos Position relative to the center the force is applied at.
 * @param rForce Force to apply.
 */
void PenaltyHangDynamics::Parts::applyForce(const sead::Vector3f& rPos,
                                            const sead::Vector3f& rForce) {
    mForce += rForce;
    mTorque += (rPos - mPos).cross(rForce);
}

/**
 * Integrates the accumulated force and torque.
 * @param deltaTime Time step.
 */
void PenaltyHangDynamics::Parts::update(f32 deltaTime) {
    f32 invMass = 1.0f / calcMass();
    mVelocity += mForce * invMass * deltaTime;
    mPos += mVelocity * deltaTime;
    sead::Vector3f localTorque;
    localTorque.setRotated(mInvQuat, mTorque);
    f32 invInertia = 1.0f / (0.4f * mRadius * mRadius);
    mLocalOmega = (mLocalOmega + localTorque * invInertia * deltaTime) * mAngularDamping;
    mWorldOmega.setRotated(mQuat, mLocalOmega);
    sead::QuatCalcCommon<f32>::applyAngularVelocity(mQuat, mWorldOmega, deltaTime);
    mInvQuat.setInverse(mQuat);
    mForce.set(0.0f, 0.0f, 0.0f);
    mTorque.set(0.0f, 0.0f, 0.0f);
}

/**
 * Resets the position and velocity and clears the accumulated force.
 * @param rPos New position.
 * @param rVelocity New velocity.
 */
void PenaltyHangDynamics::Parts::resetState(const sead::Vector3f& rPos,
                                            const sead::Vector3f& rVelocity) {
    mPos.set(rPos);
    mVelocity.set(rVelocity);
    mForce.set(0.0f, 0.0f, 0.0f);
}

/**
 * Constructs a chain of spheres.
 * @param num Number of spheres.
 * @param rUnused Unused.
 */
PenaltyHangDynamics::PenaltyHangDynamics(s32 num, const sead::Vector3f& rUnused) {
    mParts.allocBuffer(num, nullptr);

    for (s32 i = 0; i < num; i++) {
        mParts.pushBack(new Parts);
    }
}

/**
 * Places all spheres in a line hanging down from a position.
 * @param rTrans Position of the first sphere.
 */
void PenaltyHangDynamics::initTrans(const sead::Vector3f& rTrans) {
    s32 partsNum = mParts.size();

    for (s32 i = 0; i < partsNum; i++) {
        Parts* parts = mParts[i];
        parts->mPos.set(rTrans - parts->mRadius * sead::Vector3f::ey * static_cast<f32>(i));
    }
}

/**
 * Applies gravity to all spheres except the first one.
 * @param rGravity Gravity acceleration.
 */
void PenaltyHangDynamics::applyGravityAllParts(const sead::Vector3f& rGravity) {
    s32 partsNum = mParts.size();

    for (s32 i = 1; i < partsNum; i++) {
        Parts* parts = mParts(i);
        parts->mForce += rGravity * parts->calcMass();
    }
}

/**
 * Pushes the spheres out of a colliding sphere.
 * @param rCenter Center of the colliding sphere.
 * @param rVelocity Velocity of the colliding sphere.
 * @param radius Radius of the colliding sphere.
 * @param rate Strength of the penalty force.
 */
void PenaltyHangDynamics::collideSphere(const sead::Vector3f& rCenter,
                                        const sead::Vector3f& rVelocity, f32 radius, f32 rate) {
    s32 partsNum = mParts.size();

    for (s32 i = 0; i < partsNum; i++) {
        Parts* parts = mParts[i];
        sead::Vector3f diff = rCenter - parts->mPos;
        f32 partsRadius = parts->mRadius;
        sead::Vector3f dir;
        f32 distance = 0.0f;
        separateScalarAndDirection(&distance, &dir, diff);
        f32 collideDistance = partsRadius + radius;

        if (collideDistance < distance) {
            continue;
        }

        f32 depth = collideDistance - distance;
        sead::Vector3f contactPos = partsRadius * dir;
        sead::Vector3f relVelocity =
            rVelocity - parts->mVelocity + contactPos.cross(parts->mWorldOmega);
        sead::Vector3f force = depth * relVelocity * rate - depth * dir * rate;
        parts->applyForce(contactPos, force);
    }
}

/**
 * Applies the connection forces between the spheres and updates them.
 * @param deltaTime Time step.
 * @param rRootPos Position of the first sphere.
 */
void PenaltyHangDynamics::update(f32 deltaTime, const sead::Vector3f& rRootPos) {
    s32 partsNum = mParts.size();

    for (s32 i = 0; i < partsNum - 1; i++) {
        Parts* parts = mParts[i];
        Parts* nextParts = mParts[i + 1];
        sead::Vector3f up;
        calcQuatUp(&up, parts->mQuat);
        sead::Vector3f nextUp;
        calcQuatUp(&nextUp, nextParts->mQuat);
        sead::Vector3f bottomPos = parts->mPos - parts->mRadius * up;
        sead::Vector3f nextTopPos = nextParts->mRadius * nextUp + nextParts->mPos;
        sead::Vector3f diff = nextParts->mPos - bottomPos;
        sead::Vector3f nextDiff = parts->mPos - nextTopPos;
        sead::Vector3f dir;
        sead::Vector3f nextDir;
        f32 distance;
        f32 nextDistance;
        separateScalarAndDirection(&distance, &dir, diff);
        separateScalarAndDirection(&nextDistance, &nextDir, nextDiff);

        if (nextParts->mRadius < distance) {
            parts->applyForce(bottomPos, (distance - nextParts->mRadius) * dir);
        }

        if (parts->mRadius < nextDistance) {
            nextParts->applyForce(nextTopPos, (nextDistance - parts->mRadius) * nextDir);
        }
    }

    mParts[0]->resetState(rRootPos, sead::Vector3f::zero);

    for (s32 i = 1; i < partsNum; i++) {
        mParts[i]->update(deltaTime);
    }
}

}  // namespace al
