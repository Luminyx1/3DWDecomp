#include "Library/Joint/JointLookAtController.hpp"

#include "Library/Joint/JointControllerKeeper.hpp"
#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Copies a vector with a plain member-wise copy.
 * @param pDst Destination vector.
 * @param rSrc Source vector.
 */
static void copyVector(sead::Vector3f* pDst, const sead::Vector3f& rSrc) {
    static_cast<sead::BaseVec3<f32>&>(*pDst) = rSrc;
}

/**
 * Copies a quaternion with a plain member-wise copy.
 * @param pDst Destination quaternion.
 * @param rSrc Source quaternion.
 */
static void copyQuat(sead::Quatf* pDst, const sead::Quatf& rSrc) {
    static_cast<sead::BaseQuat<f32>&>(*pDst) = rSrc;
}

/**
 * Constructs a controller that turns its joints toward a requested position.
 * @param maxJoints Maximum number of joints.
 * @param pBaseMtx Base matrix of the owning actor.
 */
JointLookAtController::JointLookAtController(s32 maxJoints, const sead::Matrix34f* pBaseMtx)
    : mBaseMtx(pBaseMtx) {
    mJointInfos.allocBuffer(maxJoints, nullptr);
}

/**
 * Rotates a set of local axes by a matrix.
 * @param pAxes Receives the side, up and front axes.
 * @param rMtx Matrix to apply.
 * @param rSide Local side axis.
 * @param rUp Local up axis.
 * @param rFront Local front axis.
 */
static void calcRotatedAxes(sead::Vector3f* pAxes, const sead::Matrix34f& rMtx,
                            const sead::Vector3f& rSide, const sead::Vector3f& rUp,
                            const sead::Vector3f& rFront) {
    sead::Vector3f front = rFront;
    sead::Vector3f up = rUp;
    sead::Vector3f side = rSide;
    pAxes[2].setRotated(rMtx, front);
    pAxes[1].setRotated(rMtx, up);
    pAxes[0].setRotated(rMtx, side);
}

/**
 * Projects a direction onto a set of axes.
 * @param pOut Receives the local direction.
 * @param pAxes Side, up and front axes.
 * @param rDir Direction to project.
 */
static void calcLocalDir(sead::Vector3f* pOut, const sead::Vector3f* pAxes,
                         const sead::Vector3f& rDir) {
    pOut->x = rDir.dot(pAxes[0]);
    pOut->y = rDir.dot(pAxes[1]);
    pOut->z = rDir.dot(pAxes[2]);
}

/**
 * Applies a rotation expressed in a set of local axes to a joint matrix.
 * @param pMtx Joint matrix to modify.
 * @param rQuat Rotation in the local axes.
 * @param rSide Local side axis.
 * @param rUp Local up axis.
 * @param rFront Local front axis.
 */
static void rotateByLocalAxes(sead::Matrix34f* pMtx, const sead::Quatf& rQuat,
                              const sead::Vector3f& rSide, const sead::Vector3f& rUp,
                              const sead::Vector3f& rFront) {
    sead::Matrix34f axisMtx;
    axisMtx.makeIdentity();
    axisMtx.setBase(0, rSide);
    axisMtx.setBase(1, rUp);
    axisMtx.setBase(2, rFront);

    sead::Quatf axisQuat;
    axisMtx.toQuat(axisQuat);
    sead::Quatf invAxisQuat;
    invAxisQuat.setInverse(axisQuat);

    sead::Quatf rotateQuat = axisQuat * rQuat * invAxisQuat;
    sead::Matrix34f rotateMtx;
    rotateMtx.fromQuat(rotateQuat);
    pMtx->setMul(*pMtx, rotateMtx);
}

/**
 * Turns the joint toward the requested position within its yaw and pitch limits.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointLookAtController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    bool isPaused = isPausedJointControllers();

    for (s32 i = 0; i < mJointInfos.size(); i++) {
        JointLookAtInfo& info = mJointInfos[i];

        if (info.jointIndex != jointIndex) {
            continue;
        }

        if (info.isInvalid) {
            return;
        }

        sead::Vector3f trans;
        pMtx->getTranslation(trans);
        sead::Vector3f dir = mTargetPos;
        dir -= trans;

        if (normalizeOrZero(&dir)) {
            return;
        }

        f32 judgeLimit = info.isLookAt ? 0.0f : 0.1f;

        sead::Vector3f judgeFront;

        if (info.judgeMtx != nullptr) {
            judgeFront.setRotated(*info.judgeMtx, info.judgeFront);
        } else if (!info.isNoJudge) {
            mBaseMtx->getBase(judgeFront, 2);
        }

        sead::Vector3f prevJudgeFront = info.prevJudgeFront;
        info.prevJudgeFront = judgeFront;

        if (sead::Mathf::abs(calcAngleOnPlaneDegree(prevJudgeFront, info.prevJudgeFront,
                                                    sead::Vector3f::ey)) > 75.0f) {
            info.targetQuat.makeUnit();
            info.currentQuat.makeUnit();
            return;
        }

        sead::Quatf targetQuat;

        if (!mIsLookAt) {
            info.isLookAt = false;
            targetQuat.makeUnit();
        } else if (!info.isNoJudge && !(judgeLimit < judgeFront.dot(dir))) {
            info.isLookAt = false;

            if (mIsKeepTargetQuat) {
                copyQuat(&targetQuat, info.targetQuat);
            } else {
                targetQuat.makeUnit();
            }
        } else {
            const sead::Matrix34f* judgeBaseMtx;
            sead::Vector3f judgeAxes[3];

            if (info.isNoJudge) {
                judgeBaseMtx = pMtx;
                sead::Vector3f side = info.localSide;
                sead::Vector3f up = info.localUp;
                sead::Vector3f front = info.localFront;
                judgeAxes[0].setRotated(*pMtx, side);
                judgeAxes[1].setRotated(*pMtx, up);
                judgeAxes[2].setRotated(*pMtx, front);
            } else if (info.judgeMtx != nullptr) {
                judgeBaseMtx = info.judgeMtx;
                judgeAxes[2].setRotated(*judgeBaseMtx, info.judgeFront);
                judgeAxes[1].setRotated(*judgeBaseMtx, info.judgeUp);
                judgeAxes[0].setRotated(*judgeBaseMtx, info.judgeSide);
            } else {
                judgeBaseMtx = mBaseMtx;
                mBaseMtx->getBase(judgeAxes[0], 0);
                mBaseMtx->getBase(judgeAxes[1], 1);
                mBaseMtx->getBase(judgeAxes[2], 2);
            }

            sead::Vector3f localDir;
            calcLocalDir(&localDir, judgeAxes, dir);

            sead::Vector3f jointAxes[3];
            calcRotatedAxes(jointAxes, *pMtx, info.localSide, info.localUp, info.localFront);

            sead::Vector2f pitchYaw;
            calcSphericalPolarCoordPY(&pitchYaw, localDir, sead::Vector3f::ez,
                                      sead::Vector3f::ey);
            pitchYaw.y = sead::Mathf::clamp(pitchYaw.y, info.yawRange.x, info.yawRange.y);
            pitchYaw.x = sead::Mathf::clamp(pitchYaw.x, info.pitchRange.x, info.pitchRange.y);
            info.isLookAt = true;

            sead::Quatf yawQuat;
            yawQuat.setAxisRadian(sead::Vector3f::ey, pitchYaw.y);
            sead::Quatf pitchQuat;
            pitchQuat.setAxisRadian(-sead::Vector3f::ex, pitchYaw.x);
            sead::Quatf lookQuat = yawQuat * pitchQuat;

            sead::Vector3f lookFront;
            calcQuatFront(&lookFront, lookQuat);

            sead::Vector3f lookDir;

            if (info.isNoJudge) {
                lookDir = info.localSide * lookFront.x + info.localUp * lookFront.y +
                          info.localFront * lookFront.z;
            } else if (info.judgeMtx != nullptr) {
                lookDir = info.judgeSide * lookFront.x + info.judgeUp * lookFront.y +
                          info.judgeFront * lookFront.z;
            } else {
                lookDir = lookFront;
            }

            sead::Vector3f worldDir;
            worldDir.setRotated(*judgeBaseMtx, lookDir);

            sead::Vector3f jointDir;
            calcLocalDir(&jointDir, jointAxes, worldDir);

            if (normalizeOrZero(&jointDir)) {
                return;
            }

            if (isParallelDirection(jointDir, sead::Vector3f::ey)) {
                makeQuatFrontSide(&targetQuat, jointDir, sead::Vector3f::ex);
            } else {
                makeQuatFrontUp(&targetQuat, jointDir, sead::Vector3f::ey);
            }
        }

        copyQuat(&info.targetQuat, targetQuat);

        if (!isPaused) {
            info.currentQuat.slerpTo(info.currentQuat, targetQuat, info.rate);
        }

        rotateByLocalAxes(pMtx, info.currentQuat, info.localSide, info.localUp, info.localFront);
        return;
    }
}

/**
 * Adds a joint judged against the actor's base matrix.
 * @param jointIndex Index of the joint.
 * @param rate Interpolation rate toward the target rotation.
 * @param rYawRange Yaw limits in degrees.
 * @param rPitchRange Pitch limits in degrees.
 * @param rLocalFront Local front direction of the joint.
 * @param rLocalUp Local up direction of the joint.
 */
void JointLookAtController::appendJoint(s32 jointIndex, f32 rate, const sead::Vector2f& rYawRange,
                                        const sead::Vector2f& rPitchRange,
                                        const sead::Vector3f& rLocalFront,
                                        const sead::Vector3f& rLocalUp) {
    appendJointLocalJudge(jointIndex, rate, rYawRange, rPitchRange, rLocalFront, rLocalUp, nullptr,
                          sead::Vector3f(0.0f, 0.0f, 1.0f), sead::Vector3f(0.0f, 1.0f, 0.0f),
                          false);
}

/**
 * Adds a joint with its own judge matrix and axes.
 * @param jointIndex Index of the joint.
 * @param rate Interpolation rate toward the target rotation.
 * @param rYawRange Yaw limits in degrees.
 * @param rPitchRange Pitch limits in degrees.
 * @param rLocalFront Local front direction of the joint.
 * @param rLocalUp Local up direction of the joint.
 * @param pJudgeMtx Matrix used to judge whether the target is in front, or nullptr for the base.
 * @param rJudgeFront Front direction in the judge matrix.
 * @param rJudgeUp Up direction in the judge matrix.
 * @param isNoJudge Whether to skip the in-front judge and use the joint itself.
 */
void JointLookAtController::appendJointLocalJudge(
    s32 jointIndex, f32 rate, const sead::Vector2f& rYawRange, const sead::Vector2f& rPitchRange,
    const sead::Vector3f& rLocalFront, const sead::Vector3f& rLocalUp,
    const sead::Matrix34f* pJudgeMtx, const sead::Vector3f& rJudgeFront,
    const sead::Vector3f& rJudgeUp, bool isNoJudge) {
    JointLookAtInfo info;
    info.jointIndex = jointIndex;
    info.rate = rate;
    info.yawRange.set(sead::Mathf::deg2rad(rYawRange.x), sead::Mathf::deg2rad(rYawRange.y));
    info.pitchRange.set(sead::Mathf::deg2rad(rPitchRange.x), sead::Mathf::deg2rad(rPitchRange.y));
    copyVector(&info.localFront, rLocalFront);
    copyVector(&info.localUp, rLocalUp);
    info.localSide.setCross(rLocalUp, rLocalFront);
    normalizeOrZero(&info.localSide);
    info.currentQuat.makeUnit();
    info.judgeMtx = pJudgeMtx;
    copyVector(&info.judgeFront, rJudgeFront);
    copyVector(&info.judgeUp, rJudgeUp);
    info.judgeSide.setCross(rJudgeUp, rJudgeFront);
    normalizeOrZero(&info.judgeSide);
    info.isNoJudge = isNoJudge;

    appendJointId(jointIndex);
    mJointInfos.pushBack(info);
}

/**
 * Adds a joint that always looks without judging whether the target is in front.
 * @param jointIndex Index of the joint.
 * @param rate Interpolation rate toward the target rotation.
 * @param rYawRange Yaw limits in degrees.
 * @param rPitchRange Pitch limits in degrees.
 * @param rLocalFront Local front direction of the joint.
 * @param rLocalUp Local up direction of the joint.
 */
void JointLookAtController::appendJointNoJudge(s32 jointIndex, f32 rate,
                                               const sead::Vector2f& rYawRange,
                                               const sead::Vector2f& rPitchRange,
                                               const sead::Vector3f& rLocalFront,
                                               const sead::Vector3f& rLocalUp) {
    appendJointLocalJudge(jointIndex, rate, rYawRange, rPitchRange, rLocalFront, rLocalUp, nullptr,
                          sead::Vector3f(0.0f, 0.0f, 1.0f), sead::Vector3f(0.0f, 1.0f, 0.0f),
                          true);
}

/**
 * Requests the joints to look at a position.
 * @param rTargetPos Position to look at.
 */
void JointLookAtController::requestJointLookAt(const sead::Vector3f& rTargetPos) {
    if (!mIsValid) {
        return;
    }

    mIsRequestLookAt = true;
    mTargetPos.set(rTargetPos);
}

/**
 * Disables one joint.
 * @param jointIndex Index of the joint.
 * @return Whether the joint was found.
 */
bool JointLookAtController::invalidJoint(s32 jointIndex) {
    for (s32 i = 0; i < mJointInfos.size(); i++) {
        JointLookAtInfo& info = mJointInfos[i];

        if (info.jointIndex == jointIndex) {
            info.isInvalid = true;
            return true;
        }
    }

    return false;
}

/**
 * Enables every joint.
 */
void JointLookAtController::validAllJoint() {
    s32 size = mJointInfos.size();

    for (s32 i = 0; i < size; i++) {
        mJointInfos[i].isInvalid = false;
    }
}

}  // namespace al
