#include "Library/Play/Placement/PlacementId.hpp"

#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an empty placement id.
 */
PlacementId::PlacementId() = default;

/**
 * Constructs a placement id from its parts.
 * @param pId object id
 * @param pLayerConfigName layer name
 * @param pUnitConfigName unit config name of the zone
 * @param pZoneId zone id
 */
PlacementId::PlacementId(const char* pId, const char* pLayerConfigName, const char* pUnitConfigName,
                         const char* pZoneId)
    : mPlacementID(pId), mLayerConfigName(pLayerConfigName), mUnitConfigName(pUnitConfigName),
      mZoneID(pZoneId) {}

/**
 * Reads the placement id from placement info.
 * @param rInfo placement info
 * @return true if the object has an id
 */
bool PlacementId::init(const PlacementInfo& rInfo) {
    mPlacementID = nullptr;
    mLayerConfigName = nullptr;
    mUnitConfigName = nullptr;
    mZoneID = nullptr;
    mCommonID = nullptr;

    rInfo.getPlacementIter().tryGetStringByKey(&mCommonID, "CommonId");
    rInfo.getPlacementIter().tryGetStringByKey(&mLayerConfigName, "LayerConfigName");
    rInfo.getZoneIter().tryGetStringByKey(&mUnitConfigName, "UnitConfigName");
    rInfo.getZoneIter().tryGetStringByKey(&mZoneID, "Id");
    return rInfo.getPlacementIter().tryGetStringByKey(&mPlacementID, "Id");
}

/**
 * Compares with another placement id.
 * @param rOther id to compare with
 * @return true if both ids refer to the same object
 */
bool PlacementId::isEqual(const PlacementId& rOther) const {
    if (mCommonID != nullptr) {
        return (rOther.mCommonID != nullptr) && isEqualString(mCommonID, rOther.mCommonID);
    }

    if (rOther.mCommonID != nullptr) {
        return false;
    }

    if (mUnitConfigName != nullptr) {
        if (rOther.mUnitConfigName == nullptr || !isEqualString(mUnitConfigName, rOther.mUnitConfigName) ||
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

    return (mPlacementID != nullptr) && rOther.mPlacementID != nullptr &&
           isEqualString(mPlacementID, rOther.mPlacementID);
}

/**
 * Compares two placement ids.
 * @param rId first id
 * @param rOther second id
 * @return true if both ids refer to the same object
 */
bool PlacementId::isEqual(const PlacementId& rId, const PlacementId& rOther) {
    return rId.isEqual(rOther);
}
}  // namespace al
