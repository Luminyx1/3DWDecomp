#include "Player/PlayerActionConditionNoWaterSurface.hpp"
#include "Player/IUsePlayerWaterSurfaceInfo.hpp"

/**
 * Holds when there is no water surface near the player.
 * @param pWaterSurfaceInfo water surface finder
 */
PlayerActionConditionNoWaterSurface::PlayerActionConditionNoWaterSurface(const IUsePlayerWaterSurfaceInfo* pWaterSurfaceInfo) : mWaterSurfaceInfo(pWaterSurfaceInfo) {}

/**
 * @return whether there is no water surface
 */
bool PlayerActionConditionNoWaterSurface::check() {
    return !mWaterSurfaceInfo->isWaterSurfaceExist();
}
