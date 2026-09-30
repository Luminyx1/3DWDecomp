#include <cstring>

#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
/**
 * Calculates the nearest position on the surface of a cube area.
 * @param pOut output position
 * @param pAreaObj area to check
 * @param rPos position to check
 */
void calcNearestAreaObjEdgePos(sead::Vector3f* pOut, const AreaObj* pAreaObj,
                               const sead::Vector3f& rPos) {
    sead::Vector3f scale;
    tryGetScale(&scale, *pAreaObj->mPlacementInfo);
    sead::Vector3f center;
    tryGetTrans(&center, *pAreaObj->mPlacementInfo);

    sead::Vector3f side;
    sead::Vector3f up;
    sead::Vector3f front;
    pAreaObj->_28.getBase(side, 0);
    pAreaObj->_28.getBase(up, 1);
    pAreaObj->_28.getBase(front, 2);
    center += up * 500.0f * scale.y;

    sead::Vector3f diff = rPos - center;
    bool isOutside = false;

    f32 sideDot = diff.dot(side);
    side *= sideDot;
    f32 sideDist = sead::Mathf::abs(sideDot);
    if (sideDist > scale.x * 500.0f) {
        setLength(&side, scale.x * 500.0f);
        isOutside = true;
    }

    f32 upDot = diff.dot(up);
    up *= upDot;
    f32 upDist = sead::Mathf::abs(upDot);
    if (upDist > scale.y * 500.0f) {
        setLength(&up, scale.y * 500.0f);
        isOutside = true;
    }

    f32 frontDot = diff.dot(front);
    front *= frontDot;
    f32 frontDist = sead::Mathf::abs(frontDot);
    if (frontDist > scale.z * 500.0f) {
        setLength(&front, scale.z * 500.0f);
    } else if (!isOutside) {
        s32 axis = sideDist > upDist ? (sideDist > frontDist ? 0 : 2) : (upDist > frontDist ? 1 : 2);
        switch (axis) {
        case 0:
            setLength(&side, scale.x * 500.0f);
            break;
        case 1:
            setLength(&up, scale.y * 500.0f);
            break;
        case 2:
            setLength(&front, scale.z * 500.0f);
            break;
        }
    }

    *pOut = side + up + front + center;
}

/**
 * Checks a line segment against the shape of an area.
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @param pAreaObj area to check
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @return true if the segment hits the shape
 */
bool checkAreaObjCollisionByArrow(sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                                  const AreaObj* pAreaObj, const sead::Vector3f& rStart,
                                  const sead::Vector3f& rEnd) {
    return pAreaObj->mShape->checkArrowCollision(pHitPos, pNormal, rStart, rEnd);
}

/**
 * Finds an area of a group by its placement id.
 * @param pAreaUser area user
 * @param pGroupName name of the group
 * @param pName placement id of the area
 * @return the area, or nullptr if it doesn't exist
 */
AreaObj* tryFindAreaObjByName(const IUseAreaObj* pAreaUser, const char* pGroupName,
                              const char* pName) {
    AreaObjGroup* group = pAreaUser->getAreaObjDirector()->getAreaObjGroup(pGroupName);
    PlacementId placementId;
    if (!group) {
        return nullptr;
    }

    for (u32 i = 0; i < group->mNumAreas; i++) {
        AreaObj* areaObj = group->getAreaObj(i);
        if (tryGetPlacementID(&placementId, *areaObj->mPlacementInfo) &&
            strcmp(placementId.mPlacementID, pName) == 0) {
            return areaObj;
        }
    }

    return nullptr;
}
}  // namespace al
