#include "Library/Stage/StageInfo.hpp"

#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {
/**
 * Constructs the info of a stage or zone.
 * @param pResource resource of the stage
 * @param rPlacementIter placement data of the stage
 * @param rZoneIter placement data of the zone in its parent
 * @param pName name of the stage
 * @param pParentInfo placement info of the parent zone, or nullptr
 * @param id zone id
 */
StageInfo::StageInfo(Resource* pResource, const ByamlIter& rPlacementIter,
                     const ByamlIter& rZoneIter, const char* pName, PlacementInfo* pParentInfo,
                     s32 id)
    : mResource(pResource), mName(pName) {
    mPlacementInfo = new PlacementInfo();
    mPlacementInfo->set(rPlacementIter, rZoneIter, pParentInfo, id);
}
}  // namespace al
