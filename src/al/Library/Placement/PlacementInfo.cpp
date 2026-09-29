#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {
    /** @brief Creates placement info with empty iterators and no parent. */
    PlacementInfo::PlacementInfo() {}

    /**
     * @brief Sets every member of the placement info.
     * @param rPlacementIter The iterator over the object's placement data.
     * @param rZoneIter The iterator over the zone the object is placed in.
     * @param pParent The parent placement info, or nullptr.
     * @param idx The index stored at _28 (-1 when unset).
     */
    void PlacementInfo::set(const ByamlIter& rPlacementIter, const ByamlIter& rZoneIter, PlacementInfo* pParent, s32 idx) {
        placementIter = rPlacementIter;
        zoneIter = rZoneIter;
        _20 = pParent;
        _28 = idx;
    }
};
