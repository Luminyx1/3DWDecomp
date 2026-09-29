#include "Library/Stage/StageInfo.hpp"

#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {
/**
 * @brief Constructs the info for one stage (zone) and its placement data.
 * @param pResource The resource archive of the stage.
 * @param rPlacementIter The iterator over the stage's placement data.
 * @param rZoneIter The iterator over the zone entry that placed this stage.
 * @param pName The name of the stage.
 * @param pParentInfo The placement info of the parent stage, or nullptr.
 * @param id The id of the stage.
 */
StageInfo::StageInfo(Resource* pResource, const ByamlIter& rPlacementIter,
                     const ByamlIter& rZoneIter, const char* pName, PlacementInfo* pParentInfo,
                     s32 id)
    : mResource(pResource), mPlacementInfo(nullptr), mName(pName) {
    mPlacementInfo = new PlacementInfo();
    mPlacementInfo->set(rPlacementIter, rZoneIter, pParentInfo, id);
}

/**
 * @brief Gets the iterator over the stage's placement data.
 * @return The placement iterator.
 */
const ByamlIter& StageInfo::getPlacementIter() const {
    return mPlacementInfo->placementIter;
}

/**
 * @brief Gets the iterator over the zone entry that placed this stage.
 * @return The zone iterator.
 */
const ByamlIter& StageInfo::getZoneIter() const {
    return mPlacementInfo->zoneIter;
}

/**
 * @brief Gets the placement info of the parent stage.
 * @return The parent's placement info, or nullptr.
 */
PlacementInfo* StageInfo::getParentInfo() const {
    return mPlacementInfo->_20;
}

/**
 * @brief Gets the id of the stage.
 * @return The stage id.
 */
s32 StageInfo::getID() const {
    return mPlacementInfo->_28;
}
}  // namespace al
