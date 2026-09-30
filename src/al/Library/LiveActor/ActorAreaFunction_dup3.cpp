#include "Library/LiveActor/ActorAreaFunction.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"

namespace al {
namespace {
inline AreaObj* findAreaObj(const IUseAreaObj* pAreaUser, const char* pName,
                            const sead::Vector3f& rPos) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj(pName, rPos);
}

inline bool isInWaterAreaInline(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (findAreaObj(pAreaUser, "NoWaterArea", rPos)) {
        return false;
    }

    return findAreaObj(pAreaUser, "WaterArea", rPos) != nullptr;
}
}  // namespace

/**
 * Checks whether an area target actor is inside a water area.
 * @param pActor The actor.
 * @return Whether the actor is in water.
 */
bool isInWaterArea(const LiveActor* pActor) {
    if (!isAreaTarget(pActor)) {
        return false;
    }

    const IUseAreaObj* areaUser = pActor;
    return isInWaterAreaInline(areaUser, getTrans(pActor));
}

/**
 * Checks whether a point above an area target actor is inside a water area.
 * @param pActor The actor.
 * @param offsetY The height offset of the point.
 * @return Whether the point is in water.
 */
bool isInWaterArea(const LiveActor* pActor, f32 offsetY) {
    if (!isAreaTarget(pActor)) {
        return false;
    }

    sead::Vector3f pos = getTrans(pActor) + sead::Vector3f(0.0f, offsetY, 0.0f);
    return isInWaterAreaInline(pActor, pos);
}

/**
 * Checks whether a position is inside a water area that doesn't sink.
 * @param pAreaUser The area user.
 * @param rPos The position.
 * @return Whether the position is in non-sinking water.
 */
bool isInWaterAreaNoSink(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (findAreaObj(pAreaUser, "NoWaterArea", rPos)) {
        return false;
    }

    AreaObj* area = findAreaObj(pAreaUser, "WaterArea", rPos);
    return area && area->mIsNoSinkOcean;
}

/**
 * Checks whether an area target actor is inside a water area that doesn't sink.
 * @param pActor The actor.
 * @return Whether the actor is in non-sinking water.
 */
bool isInWaterAreaNoSink(const LiveActor* pActor) {
    if (!isAreaTarget(pActor)) {
        return false;
    }

    const IUseAreaObj* areaUser = pActor;
    return isInWaterAreaNoSink(areaUser, getTrans(pActor));
}
}  // namespace al
