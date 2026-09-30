#include "Player/PlayerActionConditionInWaterSurface.hpp"
#include "Player/IUsePlayerWaterSurfaceInfo.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerFigureDirector.hpp"

/**
 * Holds when the player is close below the water surface.
 * @param pWaterSurfaceInfo water surface finder
 * @param pFigureDirector the player's power-up
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionInWaterSurface::PlayerActionConditionInWaterSurface(
    const IUsePlayerWaterSurfaceInfo* pWaterSurfaceInfo, const PlayerFigureDirector* pFigureDirector,
    const PlayerConstParam* pConstParam)
    : mWaterSurfaceInfo(pWaterSurfaceInfo), mFigureDirector(pFigureDirector), mConstParam(pConstParam) {}

/**
 * Mini Mario uses a shorter distance.
 * @return whether the surface is above the player and within the start distance
 */
bool PlayerActionConditionInWaterSurface::check() {
    if (!mWaterSurfaceInfo->isWaterSurfaceExist() || !(mWaterSurfaceInfo->getWaterSurfaceHeight() > 0.0f)) {
        return false;
    }

    f32 height = mWaterSurfaceInfo->getWaterSurfaceHeight();
    f32 startDist = mFigureDirector->getFigure() == EPlayerFigure_Mini ? mConstParam->getSwimSurfaceStartDistShort() :
                                                                        mConstParam->getSwimSurfaceStartDist();
    return height < startDist;
}
