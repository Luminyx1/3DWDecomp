#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
/**
 * Checks whether a position is inside a water area and not inside a no water area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if the position is in water
 */
bool isInWaterArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (pAreaUser->getAreaObjDirector()->getInVolumeAreaObj("NoWaterArea", rPos) != nullptr) {
        return false;
    }

    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj("WaterArea", rPos) != nullptr;
}

/**
 * Checks whether a line segment hits a water area.
 * @param pAreaUser area user
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return true if a water area is hit
 */
bool isInWaterArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rStart,
                   const sead::Vector3f& rEnd, sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj("WaterArea", rStart, rEnd,
                                                               pHitPos, pNormal) != nullptr;
}
}  // namespace al
