#pragma once

#include <prim/seadSafeString.h>

namespace al {
class Resource;
class ByamlIter;
struct PlacementInfo;

class StageInfo {
public:
    StageInfo(Resource* pResource, const ByamlIter& rPlacementIter, const ByamlIter& rZoneIter,
              const char* pName, PlacementInfo* pParentInfo, s32 id);

    const ByamlIter& getPlacementIter() const;
    const ByamlIter& getZoneIter() const;
    PlacementInfo* getParentInfo() const;
    s32 getID() const;

    Resource* getResource() const { return mResource; }
    const PlacementInfo& getPlacementInfo() const { return *mPlacementInfo; }

    Resource* mResource;
    PlacementInfo* mPlacementInfo = nullptr;
    sead::FixedSafeString<128> mName;
};
}  // namespace al
