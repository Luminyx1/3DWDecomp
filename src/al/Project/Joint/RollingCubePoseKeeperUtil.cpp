#include "Project/Joint/RollingCubePoseKeeperUtil.hpp"

#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Project/Joint/RollingCubePoseKeeper.hpp"
#include "Project/RollingCubePose.hpp"

namespace al {

/**
 * Advances a keeper to its next key.
 * @param pKeeper Keeper to advance.
 * @return False if the end was reached on a non-looping move type.
 */
bool nextRollingCubeKey(RollingCubePoseKeeper* pKeeper) {
    return pKeeper->nextKey();
}

/**
 * Resets a keeper to its first key.
 * @param pKeeper Keeper to reset.
 */
void setStartRollingCubeKey(RollingCubePoseKeeper* pKeeper) {
    pKeeper->setStart();
}

/**
 * Sets the current key of a keeper.
 * @param pKeeper Keeper to modify.
 * @param index Key index.
 */
void setRollingCubeKeyIndex(RollingCubePoseKeeper* pKeeper, s32 index) {
    pKeeper->setKeyIndex(index);
}

/**
 * Checks whether a keeper loops back to its first key.
 * @param pKeeper Keeper to check.
 * @return True for looping move types.
 */
bool isMoveTypeLoopRollingCube(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->isMoveTypeLoop();
}

/**
 * Snaps a rotation and translation onto the current key pose.
 * @param pOutQuat Rotation to snap, in place.
 * @param pOutTrans Translation to snap, in place.
 * @param pKeeper Keeper providing the current key.
 */
void fittingToCurrentKeyBoundingBox(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans,
                                    const RollingCubePoseKeeper* pKeeper) {
    pKeeper->getCurrentPose().fittingToBoundingBox(pOutQuat, pOutTrans);
}

/**
 * Calculates the rotation and translation part way from the current key to the next one.
 * @param pOutQuat Receives the rotation.
 * @param pOutTrans Receives the translation.
 * @param pKeeper Keeper providing the current key.
 * @param rQuat Start rotation.
 * @param rTrans Start translation.
 * @param rate Progress in [0, 1].
 */
void calcCurrentKeyQT(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans,
                      const RollingCubePoseKeeper* pKeeper, const sead::Quatf& rQuat,
                      const sead::Vector3f& rTrans, f32 rate) {
    pKeeper->getCurrentPose().calcRotateQT(pOutQuat, pOutTrans, rQuat, rTrans, rate);
}

/**
 * Gets the placement rotation and translation of the current key.
 * @param pOutQuat Receives the rotation, may be null.
 * @param pOutTrans Receives the translation, may be null.
 * @param pKeeper Keeper providing the current key.
 */
void getCurrentKeyQT(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans,
                     const RollingCubePoseKeeper* pKeeper) {
    const RollingCubePose& rollingCubePose = pKeeper->getCurrentPose();

    if (pOutQuat != nullptr) {
        pOutQuat->set(rollingCubePose.getQuat());
    }

    if (pOutTrans != nullptr) {
        pOutTrans->set(rollingCubePose.getTrans());
    }
}

/**
 * Gets the rotation angle from the current key to the next one.
 * @param pKeeper Keeper providing the current key.
 * @return Angle in degrees.
 */
f32 getCurrentKeyRotateDegree(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->getCurrentPose().getRotateDegree();
}

/**
 * Gets the slide offset from the current key to the next one.
 * @param pKeeper Keeper providing the current key.
 * @return Slide vector.
 */
const sead::Vector3f& getCurrentKeySlideVec(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->getCurrentPose().getSlideVec();
}

/**
 * Gets the current key index.
 * @param pKeeper Keeper to query.
 * @return Current key index.
 */
s32 getCurrentKeyIndex(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->getCurrentKeyIndex();
}

/**
 * Gets the placement info of the current key.
 * @param pKeeper Keeper providing the current key.
 * @return Placement info.
 */
const PlacementInfo& getCurrentKeyPlacementInfo(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->getCurrentPose().getPlacementInfo();
}

/**
 * Checks whether the cube rotates over an edge to reach the next key.
 * @param pKeeper Keeper providing the current key.
 * @return True for rotate movement.
 */
bool isMovementCurrentKeyRotate(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->getCurrentPose().isMovementRotate();
}

/**
 * Checks whether the cube slides to reach the next key.
 * @param pKeeper Keeper providing the current key.
 * @return True for slide movement.
 */
bool isMovementCurrentKeySlide(const RollingCubePoseKeeper* pKeeper) {
    return pKeeper->getCurrentPose().isMovementSlide();
}

/**
 * Calculates the distance from the rotation edge to the cube center, perpendicular to the edge.
 * @param pKeeper Keeper providing the current key.
 * @return Distance.
 */
f32 calcDistanceCurrentKeyRotateCenterToBoxCenter(const RollingCubePoseKeeper* pKeeper) {
    const RollingCubePose& rollingCubePose = pKeeper->getCurrentPose();

    sead::Vector3f center;
    rollingCubePose.calcBoundingBoxCenter(&center);

    sead::Vector3f distance = center - rollingCubePose.getRotateCenter();

    if (!isNearZero(rollingCubePose.getRotateAxis())) {
        verticalizeVec(&distance, rollingCubePose.getRotateAxis(), distance);
    }

    return distance.length();
}

/**
 * Calculates a bounding sphere enclosing the cube at every key.
 * @param pClippingTrans Receives the sphere center.
 * @param pClippingRadius Receives the sphere radius.
 * @param pKeeper Keeper providing the keys.
 * @param offset Extra radius.
 */
void calcRollingCubeClippingInfo(sead::Vector3f* pClippingTrans, f32* pClippingRadius,
                                 const RollingCubePoseKeeper* pKeeper, f32 offset) {
    sead::BoundBox3f cubeSize = pKeeper->getCubeSize();
    sead::Vector3f corners[8];
    corners[0].set(cubeSize.getMin().x, cubeSize.getMin().y, cubeSize.getMin().z);
    corners[1].set(cubeSize.getMax().x, cubeSize.getMin().y, cubeSize.getMin().z);
    corners[2].set(cubeSize.getMax().x, cubeSize.getMin().y, cubeSize.getMax().z);
    corners[3].set(cubeSize.getMin().x, cubeSize.getMin().y, cubeSize.getMax().z);
    corners[4].set(cubeSize.getMin().x, cubeSize.getMax().y, cubeSize.getMin().z);
    corners[5].set(cubeSize.getMax().x, cubeSize.getMax().y, cubeSize.getMin().z);
    corners[6].set(cubeSize.getMax().x, cubeSize.getMax().y, cubeSize.getMax().z);
    corners[7].set(cubeSize.getMin().x, cubeSize.getMax().y, cubeSize.getMax().z);

    sead::BoundBox3f clippingBox;
    s32 poseCount = pKeeper->getPoseCount();

    for (s32 i = 0; i < poseCount; i++) {
        const RollingCubePose& pose = pKeeper->getPose(i);

        sead::Matrix34f poseMtx;
        poseMtx.makeQT(pose.getQuat(), pose.getTrans());

        for (s32 j = 0; j < 8; j++) {
            sead::Vector3f corner;
            corner.setMul(poseMtx, corners[j]);
            clippingBox.addPoint(corner);
        }
    }

    clippingBox.getCenter(pClippingTrans);
    sead::Vector3f size = clippingBox.getMax() - clippingBox.getMin();
    *pClippingRadius = size.length() * 0.5f + offset;
}

/**
 * Calculates the landing effect matrix on the face of the cube currently facing down.
 * @param pEffectMtx Receives the effect matrix.
 * @param pKeeper Keeper providing the cube size.
 * @param rQuat Cube rotation.
 * @param rTrans Cube translation.
 */
void calcMtxLandEffect(sead::Matrix34f* pEffectMtx, const RollingCubePoseKeeper* pKeeper,
                       const sead::Quatf& rQuat, const sead::Vector3f& rTrans) {
    sead::Vector3f side;
    sead::Vector3f up;
    sead::Vector3f front;
    calcQuatLocalAxisAll(rQuat, &side, &up, &front);
    sead::Vector3f landUp = up;
    sead::Vector3f landFront = front;
    sead::Vector3f offset;

    if (sead::Mathf::abs(side.y) > sead::Mathf::abs(up.y)) {
        if (sead::Mathf::abs(side.y) > sead::Mathf::abs(front.y)) {
            landUp = side.y > 0.0f ? side : -side;
            landFront.set(front);
            const sead::BoundBox3f& box = pKeeper->getCubeSize();
            offset = landUp * ((box.getMax().x - box.getMin().x) * -0.5f);
        } else {
            landUp = front.y > 0.0f ? front : -front;
            landFront.set(side);
            const sead::BoundBox3f& box = pKeeper->getCubeSize();
            offset = landUp * ((box.getMax().z - box.getMin().z) * -0.5f);
        }
    } else if (sead::Mathf::abs(up.y) > sead::Mathf::abs(front.y)) {
        landUp = up.y > 0.0f ? up : -up;
        landFront.set(front);
        const sead::BoundBox3f& box = pKeeper->getCubeSize();
        offset = landUp * ((box.getMax().y - box.getMin().y) * -0.5f);
    } else {
        landUp = front.y > 0.0f ? front : -front;
        landFront.set(side);
        const sead::BoundBox3f& box = pKeeper->getCubeSize();
        offset = landUp * ((box.getMax().z - box.getMin().z) * -0.5f);
    }

    sead::Vector3f center;
    pKeeper->calcBoundingBoxCenter(&center, rQuat, rTrans);
    sead::Vector3f pos = offset + center;
    makeMtxUpFrontPos(pEffectMtx, landUp, landFront, pos);
}

}  // namespace al
