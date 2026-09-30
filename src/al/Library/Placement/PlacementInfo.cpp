#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {
/**
 * Constructs empty placement info.
 */
PlacementInfo::PlacementInfo() : _20(nullptr), _28(-1) {}

/**
 * Sets the placement and zone iterators.
 * @param rPlacementIter placement data iterator
 * @param rZoneIter zone data iterator
 * @param pParent parent placement info
 * @param index index in the parent
 */
void PlacementInfo::set(const ByamlIter& rPlacementIter, const ByamlIter& rZoneIter,
                        PlacementInfo* pParent, s32 index) {
    placementIter = rPlacementIter;
    zoneIter = rZoneIter;
    _20 = pParent;
    _28 = index;
}
}  // namespace al
