#include "Project/Block/BlockRailShapeTerminater.hpp"

#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Block/BlockRailLink.hpp"
#include "Project/Block/BlockRailParts.hpp"
#include "Project/Block/BlockRailPartsGroup.hpp"
#include "Project/Block/BlockRailRider.hpp"
#include "Project/Block/BlockRailShapeStraight.hpp"
#include "Project/Block/BlockRailUtil.hpp"

namespace al {
/**
 * Constructs a terminater block rail shape.
 * @param pName shape name
 */
BlockRailShapeTerminater::BlockRailShapeTerminater(const char* pName) : BlockRailShape(pName) {}

/**
 * Initializes the position and front direction.
 * @param rQuat rotation
 * @param rTrans translation
 * @param rIter shape parameters
 */
void BlockRailShapeTerminater::init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                                    const ByamlIter& rIter) {
    sead::Vector3f localTrans = sead::Vector3f::zero;
    tryGetByamlV3f(&localTrans, rIter, "Trans");
    sead::Vector3f frontDir = sead::Vector3f::ez;
    tryGetByamlV3f(&frontDir, rIter, "FrontDir");

    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);
    mPos.setMul(mtx, localTrans);
    mDir.setRotated(rQuat, frontDir);
}

/**
 * Calculates the position.
 * @param pPos output position
 * @param rate rate on the shape
 */
void BlockRailShapeTerminater::calcPos(sead::Vector3f* pPos, f32 rate) const {
    *pPos = mPos;
}

/**
 * Calculates the direction.
 * @param pDir output direction
 * @param rate rate on the shape
 */
void BlockRailShapeTerminater::calcDir(sead::Vector3f* pDir, f32 rate) const {
    *pDir = mDir;
}

/**
 * Calculates the nearest point on the shape.
 * @param pPos output position
 * @param pRate output rate
 * @param rPos position
 */
void BlockRailShapeTerminater::calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                                                const sead::Vector3f& rPos) const {
    *pPos = mPos;
    *pRate = 0.0f;
}

/**
 * Checks if this is a terminater.
 * @return true
 */
bool BlockRailShapeTerminater::isTerminate() const {
    return true;
}

/**
 * Calculates the offset from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailShapeTerminater::calcOffset(const sead::Vector3f& rBaseTrans) {
    mOffset = mPos - rBaseTrans;
}

/**
 * Updates the position from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailShapeTerminater::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {
    mPos = rBaseTrans + mOffset;
}

/**
 * Puts a rider on the nearest rail of a parts group.
 * @param pRider rider
 * @param pGroup parts group
 * @param rPos position
 */
void setBlockRailRiderToNearestPos(BlockRailRider* pRider, const BlockRailPartsGroup* pGroup,
                                   const sead::Vector3f& rPos) {
    BlockRailLink* link = nullptr;
    f32 rate = 0.0f;
    calcNearBlockRailLinkAndCoord(&link, &rate, pGroup, rPos);
    pRider->setRailPart(link, rate * link->getTotalLength());
}

/**
 * Finds the nearest rail link of a parts group.
 * @param pLink output link
 * @param pRate output rate on the link
 * @param pGroup parts group
 * @param rPos position
 */
void calcNearBlockRailLinkAndCoord(BlockRailLink** pLink, f32* pRate,
                                   const BlockRailPartsGroup* pGroup, const sead::Vector3f& rPos) {
    BlockRailLink* nearLink = nullptr;
    f32 nearRate = 0.0f;
    f32 minDistance = sead::Mathf::maxNumber();
    s32 partsNum = pGroup->getPartsNum();

    for (s32 i = 0; i < partsNum; i++) {
        BlockRailParts* parts = pGroup->getParts(i);
        s32 linkNum = parts->getLinkNum();

        for (s32 j = 0; j < linkNum; j++) {
            BlockRailLink* link = parts->getLink(j);
            sead::Vector3f nearPos = sead::Vector3f::zero;
            f32 rate = 0.0f;
            link->calcNearestParam(&nearPos, &rate, rPos);
            f32 distance = (nearPos - rPos).length();

            if (distance < minDistance) {
                minDistance = distance;
                nearLink = link;
                nearRate = rate;
            }
        }
    }

    *pLink = nearLink;
    *pRate = nearRate;
}

/**
 * Finds the nearest point on the rails of a parts group.
 * @param pNearPos output position
 * @param pGroup parts group
 * @param rPos position
 */
void calcNearBlockRailPos(sead::Vector3f* pNearPos, const BlockRailPartsGroup* pGroup,
                          const sead::Vector3f& rPos) {
    BlockRailLink* link = nullptr;
    f32 rate = 0.0f;
    calcNearBlockRailLinkAndCoord(&link, &rate, pGroup, rPos);
    link->calcPos(pNearPos, rate);
}

/**
 * Calculates a rider's position.
 * @param pPos output position
 * @param pRider rider
 */
void calcPosBlockRailRider(sead::Vector3f* pPos, const BlockRailRider* pRider) {
    sead::Vector3f dir;
    pRider->calcPosAndDir(pPos, &dir);
}

/**
 * Calculates a rider's position and direction.
 * @param pPos output position
 * @param pDir output direction
 * @param pRider rider
 */
void calcPosAndDirBlockRailRider(sead::Vector3f* pPos, sead::Vector3f* pDir,
                                 const BlockRailRider* pRider) {
    pRider->calcPosAndDir(pPos, pDir);
}

/**
 * Moves a rider.
 * @param pRider rider
 * @param speed movement distance
 */
void moveBlockRailRider(BlockRailRider* pRider, f32 speed) {
    sead::Vector3f pos;
    sead::Vector3f dir;
    pRider->move(speed, &pos, &dir);
}

/**
 * Moves a rider and calculates its position.
 * @param pPos output position
 * @param pRider rider
 * @param speed movement distance
 */
void moveBlockRailRiderAndCalcPos(sead::Vector3f* pPos, BlockRailRider* pRider, f32 speed) {
    sead::Vector3f dir;
    pRider->move(speed, pPos, &dir);
}

/**
 * Moves a rider and calculates its position and direction.
 * @param pPos output position
 * @param pDir output direction
 * @param pRider rider
 * @param speed movement distance
 */
void moveBlockRailRiderAndCalcPosDir(sead::Vector3f* pPos, sead::Vector3f* pDir,
                                     BlockRailRider* pRider, f32 speed) {
    pRider->move(speed, pPos, pDir);
}

/**
 * Calculates a clipping sphere enclosing all parts of a group.
 * @param pCenter output center
 * @param pRadius output radius
 * @param pGroup parts group
 * @param rBaseCenter base sphere center
 * @param baseRadius base sphere radius
 */
void calcBlockRailClippingSphere(sead::Vector3f* pCenter, f32* pRadius,
                                 const BlockRailPartsGroup* pGroup,
                                 const sead::Vector3f& rBaseCenter, f32 baseRadius) {
    *pCenter = rBaseCenter;
    *pRadius = baseRadius;
    s32 partsNum = pGroup->getPartsNum();

    for (s32 i = 0; i < partsNum; i++) {
        BlockRailParts* parts = pGroup->getParts(i);
        calcSphereMargeSpheres(pCenter, pRadius, *pCenter, *pRadius, getTrans(parts),
                               getClippingRadius(parts));
    }
}
}  // namespace al
