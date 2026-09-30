#pragma once

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
struct PlacementInfo {
    PlacementInfo();

    void set(const ByamlIter& rPlacementIter, const ByamlIter& rZoneIter, PlacementInfo* pParent,
             s32 index);

    const ByamlIter& getPlacementIter() const { return placementIter; }
    const ByamlIter& getZoneIter() const { return zoneIter; }

    ByamlIter placementIter;
    ByamlIter zoneIter;
    PlacementInfo* _20;
    s32 _28;
};
}  // namespace al
