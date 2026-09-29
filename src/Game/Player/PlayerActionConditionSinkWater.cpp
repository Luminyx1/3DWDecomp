#include "Player/PlayerActionConditionSinkWater.hpp"
#include "Player/IUsePlayerCheckArea.hpp"
#include "Player/IUsePlayerWaterSurfaceInfo.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerFigureDirector.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player should sink.
 * @param pWaterSurfaceInfo water surface finder
 * @param pFigureDirector the player's power-up
 * @param pConstParam the player's tuning values
 * @param pCheckArea area checker
 * @param pProperty the player's physical state
 */
PlayerActionConditionSinkWater::PlayerActionConditionSinkWater(const IUsePlayerWaterSurfaceInfo* pWaterSurfaceInfo,
                                                               const PlayerFigureDirector* pFigureDirector,
                                                               const PlayerConstParam* pConstParam,
                                                               const IUsePlayerCheckArea* pCheckArea,
                                                               const PlayerProperty* pProperty)
    : mWaterSurfaceInfo(pWaterSurfaceInfo), mFigureDirector(pFigureDirector), mConstParam(pConstParam),
      mCheckArea(pCheckArea), mProperty(pProperty) {}

/**
 * Mini Mario uses a shorter distance.
 * @return whether there is no surface, the water does not let the player sink, or the surface is
 *         too far above or below
 */
bool PlayerActionConditionSinkWater::check() {
    if (!mWaterSurfaceInfo->isWaterSurfaceExist()) {
        return true;
    }
    if (mCheckArea->isInWaterNoSink(mProperty->mTrans)) {
        return true;
    }
    f32 height = mWaterSurfaceInfo->getWaterSurfaceHeight();
    f32 endDist = mFigureDirector->getFigure() == EPlayerFigure_Mini ? mConstParam->getSwimSurfaceEndDistShort() :
                                                                      mConstParam->getSwimSurfaceEndDist();
    if (height >= endDist) {
        return true;
    }
    return mWaterSurfaceInfo->getWaterSurfaceHeight() <= 0.0f;
}
