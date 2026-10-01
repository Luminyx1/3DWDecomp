#include "Library/Play/Placement/PlacementHolder.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an empty placement holder.
 */
PlacementHolder::PlacementHolder() = default;

/**
 * Constructs a placement holder from its ids.
 * @param pId object id
 * @param pUnitConfigName unit config name of the zone
 * @param pZoneId zone id
 */
PlacementHolder::PlacementHolder(const char* pId, const char* pUnitConfigName, const char* pZoneId)
    : mId(pId), mUnitConfigName(pUnitConfigName), mZoneId(pZoneId), mZoneNo(-1) {}

/**
 * Reads the ids, zone number and layer of a placement, taking the zone data from the outermost parent.
 * @param rInfo placement info to read
 * @return whether the placement has an id
 */
bool PlacementHolder::init(const PlacementInfo& rInfo) {
    mUnitConfigName = nullptr;
    mZoneNo = -1;
    mLayerId = -1;
    mId = nullptr;
    mCommonId = nullptr;
    mZoneId = nullptr;

    rInfo.getPlacementIter().tryGetStringByKey(&mCommonId, "CommonId");
    rInfo.getZoneIter().tryGetStringByKey(&mUnitConfigName, "UnitConfigName");
    rInfo.getZoneIter().tryGetStringByKey(&mZoneId, "Id");
    rInfo.getZoneIter().tryGetIntByKey(&mZoneNo, "ZoneID");
    mLayerId = tryGetLayerID(rInfo);

    PlacementInfo info = rInfo;

    while (info._20 != nullptr) {
        info = *info._20;
        info.getZoneIter().tryGetStringByKey(&mUnitConfigName, "UnitConfigName");
        info.getZoneIter().tryGetIntByKey(&mZoneNo, "ZoneID");
    }

    return rInfo.getPlacementIter().tryGetStringByKey(&mId, "Id");
}

/**
 * Copies the zone data and layer of a parent placement.
 * @param rParent parent placement holder
 */
void PlacementHolder::copyFromParent(const PlacementHolder& rParent) {
    mZoneNo = rParent.mZoneNo;
    mZoneId = rParent.mZoneId;
    mUnitConfigName = rParent.mUnitConfigName;
    mLayerId = rParent.mLayerId;
}

/**
 * Checks whether this holder refers to the same placement as another.
 * @param rOther holder to compare with
 * @return whether both refer to the same placement
 */
bool PlacementHolder::isEqual(const PlacementHolder& rOther) const {
    if (mCommonId != nullptr) {
        return rOther.mCommonId != nullptr && isEqualString(mCommonId, rOther.mCommonId);
    }

    if (rOther.mCommonId != nullptr) {
        return false;
    }

    if (mUnitConfigName != nullptr) {
        if (rOther.mUnitConfigName == nullptr || !isEqualString(mUnitConfigName, rOther.mUnitConfigName) ||
            !isEqualString(mZoneId, rOther.mZoneId)) {
            return false;
        }
    } else if (rOther.mUnitConfigName != nullptr) {
        return false;
    }

    return mId != nullptr && rOther.mId != nullptr && mLayerId == rOther.mLayerId &&
           isEqualString(mId, rOther.mId);
}

/**
 * Checks whether two holders refer to the same placement.
 * @param rHolder first holder
 * @param rOther second holder
 * @return whether both refer to the same placement
 */
bool PlacementHolder::isEqual(const PlacementHolder& rHolder, const PlacementHolder& rOther) {
    return rHolder.isEqual(rOther);
}
}  // namespace al
