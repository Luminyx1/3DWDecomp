#pragma once

#include "Library/Layout/LayoutActor.hpp"
#include "System/ScenarioInfo.hpp"

namespace al {
class LayoutInitInfo;
}

class GameDataHolder;

/**
 * @brief Layout parts showing an island's name and its collected shine count.
 * @note Only what reconstructed code needs is declared so far.
 */
class IslandCounterParts : public al::LayoutActor {
public:
    IslandCounterParts(const al::LayoutInitInfo& rInfo, const char* pName, const char* pPaneName,
                       al::LayoutActor* pParent, const char* pArchiveName);

    void setIslandName(const char16_t* pName);
    void updateShineCount(s32 islandId);
    void updateShineCountOcean(const GameDataHolder* pGameData, ScenarioInfo scenarioInfo);

private:
    u8 _121[0x130 - 0x121];
};

static_assert(sizeof(IslandCounterParts) == 0x130);
