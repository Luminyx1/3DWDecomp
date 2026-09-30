#include "Project/Joint/RollingCubePoseKeeperUtil.hpp"

#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Project/Joint/RollingCubePoseKeeper.hpp"
#include "Project/RollingCubePose.hpp"

namespace al {
void calcModelBoundingBox(sead::BoundBox3f* pBox, const LiveActor* pActor);

RollingCubePoseKeeper* createRollingCubePoseKeeper(const LiveActor* actor,
                                                   const ActorInitInfo& initInfo) {
    sead::BoundBox3f modelBoundBox;
    calcModelBoundingBox(&modelBoundBox, actor);
    return createRollingCubePoseKeeper(modelBoundBox, initInfo);
}

RollingCubePoseKeeper* createRollingCubePoseKeeper(const sead::BoundBox3f& cubeSize,
                                                   const ActorInitInfo& initInfo) {
    RollingCubePoseKeeper* keeper = new RollingCubePoseKeeper();
    keeper->setCubeSize(cubeSize);
    keeper->init(initInfo);
    return keeper;
}

bool nextRollingCubeKey(RollingCubePoseKeeper* keeper) {
    return keeper->nextKey();
}

void setStartRollingCubeKey(RollingCubePoseKeeper* keeper) {
    keeper->setStart();
}

void setRollingCubeKeyIndex(RollingCubePoseKeeper* keeper, s32 index) {
    keeper->setKeyIndex(index);
}

bool isMoveTypeLoopRollingCube(const RollingCubePoseKeeper* keeper) {
    return keeper->isMoveTypeLoop();
}

void fittingToCurrentKeyBoundingBox(sead::Quatf* outQuat, sead::Vector3f* outTrans,
                                    const RollingCubePoseKeeper* keeper) {
    keeper->getCurrentPose().fittingToBoundingBox(outQuat, outTrans);
}

void calcCurrentKeyQT(sead::Quatf* outQuat, sead::Vector3f* outTrans,
                      const RollingCubePoseKeeper* keeper, const sead::Quatf& quat,
                      const sead::Vector3f& trans, f32 rate) {
    keeper->getCurrentPose().calcRotateQT(outQuat, outTrans, quat, trans, rate);
}

void getCurrentKeyQT(sead::Quatf* outQuat, sead::Vector3f* outTrans,
                     const RollingCubePoseKeeper* keeper) {
    const RollingCubePose& rollingCubePose = keeper->getCurrentPose();
    if (outQuat)
        outQuat->set(rollingCubePose.getQuat());
    if (outTrans)
        outTrans->set(rollingCubePose.getTrans());
}

f32 getCurrentKeyRotateDegree(const RollingCubePoseKeeper* keeper) {
    return keeper->getCurrentPose().getRotateDegree();
}

const sead::Vector3f& getCurrentKeySlideVec(const RollingCubePoseKeeper* keeper) {
    return keeper->getCurrentPose().getSlideVec();
}

s32 getCurrentKeyIndex(const RollingCubePoseKeeper* keeper) {
    return keeper->getCurrentKeyIndex();
}

const PlacementInfo& getCurrentKeyPlacementInfo(const RollingCubePoseKeeper* keeper) {
    return keeper->getCurrentPose().getPlacementInfo();
}

bool isMovementCurrentKeyRotate(const RollingCubePoseKeeper* keeper) {
    return keeper->getCurrentPose().isMovementRotate();
}

bool isMovementCurrentKeySlide(const RollingCubePoseKeeper* keeper) {
    return keeper->getCurrentPose().isMovementSlide();
}

f32 calcDistanceCurrentKeyRotateCenterToBoxCenter(const RollingCubePoseKeeper* keeper) {
    const RollingCubePose& rollingCubePose = keeper->getCurrentPose();

    sead::Vector3f center;
    rollingCubePose.calcBoundingBoxCenter(&center);

    sead::Vector3f distance = center - rollingCubePose.getRotateCenter();

    if (!isNearZero(rollingCubePose.getRotateAxis()))
        verticalizeVec(&distance, rollingCubePose.getRotateAxis(), distance);

    return distance.length();
}

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
