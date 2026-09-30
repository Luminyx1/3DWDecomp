#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
/**
 * Checks whether a position is inside a player control off area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInPlayerControlOffArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj("PlayerControlOffArea", rPos) !=
           nullptr;
}
}  // namespace al
