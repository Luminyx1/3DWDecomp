#include "Project/AreaObj/AreaObj.hpp"

namespace al {
    /** @brief Constructs an empty area init info. */
    AreaInitInfo::AreaInitInfo() : mSwitchDirector(nullptr) {}

    /**
     * @brief Constructs an area init info from a placement.
     * @param rPlacementInfo The placement of the area.
     * @param pSwitchDirector The scene's stage switch director.
     */
    AreaInitInfo::AreaInitInfo(const PlacementInfo& rPlacementInfo, StageSwitchDirector* pSwitchDirector)
        : mPlacementInfo(rPlacementInfo), mSwitchDirector(pSwitchDirector) {}

    /**
     * @brief Constructs an area init info from a placement, sharing the scene data of another init info.
     * @param rPlacementInfo The placement of the area.
     * @param rInitInfo The init info to take the scene data from.
     */
    AreaInitInfo::AreaInitInfo(const PlacementInfo& rPlacementInfo, const AreaInitInfo& rInitInfo)
        : mPlacementInfo(rPlacementInfo), mSwitchDirector(rInitInfo.mSwitchDirector) {}

    /**
     * @brief Sets the placement and switch director.
     * @param rPlacementInfo The placement of the area.
     * @param pSwitchDirector The scene's stage switch director.
     */
    void AreaInitInfo::set(const PlacementInfo& rPlacementInfo, StageSwitchDirector* pSwitchDirector) {
        mPlacementInfo = rPlacementInfo;
        mSwitchDirector = pSwitchDirector;
    }
};
