#include "Library/LiveActor/ActorAreaFunction.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"

namespace al {
/**
 * Checks whether an area target actor is inside a death area.
 * @param pActor The actor.
 * @return Whether the actor is inside a death area.
 */
bool isInDeathArea(const LiveActor* pActor) {
    if (!isAreaTarget(pActor)) {
        return false;
    }
    const IUseAreaObj* areaUser = pActor;
    const sead::Vector3f& trans = getTrans(pActor);
    return areaUser->getAreaObjDirector()->getInVolumeAreaObj("DeathArea", trans) != nullptr;
}
}  // namespace al
