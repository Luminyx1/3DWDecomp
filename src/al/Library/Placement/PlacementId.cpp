#include "Library/Play/Placement/PlacementId.hpp"

#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * @brief Constructs an empty placement id.
 */
PlacementId::PlacementId() = default;

/**
 * @brief Constructs a placement id from its parts.
 * @param pId The object's id inside its placement file.
 * @param pLayerConfigName The name of the layer the object is placed on.
 * @param pUnitConfigName The unit config name of the zone the object belongs to.
 * @param pZoneId The id of the zone the object belongs to.
 */
PlacementId::PlacementId(const char* pId, const char* pLayerConfigName,
                         const char* pUnitConfigName, const char* pZoneId)
    : mPlacementID(pId), mLayerConfigName(pLayerConfigName), mUnitConfigName(pUnitConfigName),
      mZoneID(pZoneId) {}

/**
 * @brief Reads the placement id from placement info.
 * @param rInfo The placement info to read from.
 * @return True if the object's Id entry was found.
 */
bool PlacementId::init(const PlacementInfo& rInfo) {
    mPlacementID = nullptr;
    mCommonID = nullptr;
    mZoneID = nullptr;
    mUnitConfigName = nullptr;
    mLayerConfigName = nullptr;

    rInfo.placementIter.tryGetStringByKey(&mCommonID, "CommonId");
    rInfo.placementIter.tryGetStringByKey(&mLayerConfigName, "LayerConfigName");
    rInfo.zoneIter.tryGetStringByKey(&mUnitConfigName, "UnitConfigName");
    rInfo.zoneIter.tryGetStringByKey(&mZoneID, "Id");
    return rInfo.placementIter.tryGetStringByKey(&mPlacementID, "Id");
}

/**
 * @brief Checks whether this placement id refers to the same object as another.
 * @param rOther The placement id to compare against.
 * @return True if both ids refer to the same object.
 */
bool PlacementId::isEqual(const PlacementId& rOther) const {
    if (mCommonID != nullptr) {
        return rOther.mCommonID != nullptr && isEqualString(mCommonID, rOther.mCommonID);
    }

    if (rOther.mCommonID != nullptr) {
        return false;
    }

    if (mUnitConfigName != nullptr) {
        if (rOther.mUnitConfigName == nullptr ||
            !isEqualString(mUnitConfigName, rOther.mUnitConfigName) ||
            !isEqualString(mZoneID, rOther.mZoneID)) {
            return false;
        }
    } else if (rOther.mUnitConfigName != nullptr) {
        return false;
    }

    if (mLayerConfigName != nullptr) {
        if (rOther.mLayerConfigName == nullptr ||
            !isEqualString(mLayerConfigName, rOther.mLayerConfigName)) {
            return false;
        }
    } else if (rOther.mLayerConfigName != nullptr) {
        return false;
    }

    return mPlacementID != nullptr && rOther.mPlacementID != nullptr &&
           isEqualString(mPlacementID, rOther.mPlacementID);
}

/**
 * @brief Checks whether two placement ids refer to the same object.
 * @param rSelf The first placement id.
 * @param rOther The second placement id.
 * @return True if both ids refer to the same object.
 */
bool PlacementId::isEqual(const PlacementId& rSelf, const PlacementId& rOther) {
    return rSelf.isEqual(rOther);
}
}  // namespace al
