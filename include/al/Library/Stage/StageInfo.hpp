#pragma once

#include <prim/seadSafeString.hpp>

namespace al {
class Resource;
class ByamlIter;
struct PlacementInfo;

class StageInfo {
public:
    StageInfo(Resource*, const ByamlIter&, const ByamlIter&, const char*, PlacementInfo*, s32);

    const ByamlIter& getPlacementIter() const;
    const ByamlIter& getZoneIter() const;
    PlacementInfo* getParentInfo() const;
    s32 getID() const;

    Resource* mResource;               // _0
    PlacementInfo* mPlacementInfo;     // _8
    sead::FixedSafeString<128> mName;  // _10
};
}  // namespace al
