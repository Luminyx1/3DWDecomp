#pragma once

#include <math/seadVector.h>

#include "Library/Layout/LayoutActor.hpp"
#include "Library/Scene/ISceneObj.hpp"
#include "Project/Camera/Main/IUseCameraDirector_RS.hpp"

/**
 * @brief The island map layout of Bowser's Fury.
 * @note Only what reconstructed code needs is declared so far.
 */
class IslandMap : public al::LayoutActor, public al::ISceneObj, public al::IUseCamera_RS {
public:
    static void setIslandWarpEnable(const al::IUseSceneObjHolder* pUser, bool isEnable);
    void updatePlayerTrackerForBonusArea(sead::Vector3f trans);

    /**
     * @brief Set whether a player went into a bonus area.
     * @param isInBonusArea True while a player is in a bonus area.
     */
    void setIsPlayerInBonusArea(bool isInBonusArea) { mIsPlayerInBonusArea = isInBonusArea; }

private:
    u8 _138[0x1e1 - 0x138];
    bool mIsPlayerInBonusArea;  // 0x1e1
};
