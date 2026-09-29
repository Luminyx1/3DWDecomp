#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {
    /** @brief Creates placement info with empty iterators and no parent. */
    PlacementInfo::PlacementInfo() {}

    /** @brief Sets the placement and zone iterators, the parent placement and the layer index. */
    void PlacementInfo::set(const ByamlIter& rPlacementIter, const ByamlIter& rZoneIter, PlacementInfo* pParent, s32 idx) {
        placementIter = rPlacementIter;
        zoneIter = rZoneIter;
        _20 = pParent;
        _28 = idx;
    }
};
