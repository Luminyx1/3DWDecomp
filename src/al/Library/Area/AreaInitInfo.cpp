#include "Project/AreaObj/AreaObj.hpp"

namespace al {
/**
 * Constructs empty area init info.
 */
AreaInitInfo::AreaInitInfo() : mSwitchDirector(nullptr) {}

/**
 * Constructs area init info from placement info.
 * @param rInfo placement info of the area
 * @param pSwitchDirector stage switch director
 */
AreaInitInfo::AreaInitInfo(const PlacementInfo& rInfo, StageSwitchDirector* pSwitchDirector)
    : mPlacementInfo(rInfo), mSwitchDirector(pSwitchDirector) {}

/**
 * Constructs area init info from placement info, sharing the directors of other init info.
 * @param rInfo placement info of the area
 * @param rInitInfo init info to take the directors from
 */
AreaInitInfo::AreaInitInfo(const PlacementInfo& rInfo, const AreaInitInfo& rInitInfo)
    : mPlacementInfo(rInfo), mSwitchDirector(rInitInfo.mSwitchDirector) {}

/**
 * Sets the placement info and stage switch director.
 * @param rInfo placement info of the area
 * @param pSwitchDirector stage switch director
 */
void AreaInitInfo::set(const PlacementInfo& rInfo, StageSwitchDirector* pSwitchDirector) {
    mPlacementInfo = rInfo;
    mSwitchDirector = pSwitchDirector;
}
}  // namespace al
