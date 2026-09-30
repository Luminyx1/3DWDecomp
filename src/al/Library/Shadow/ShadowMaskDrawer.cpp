#include "Library/Shadow/ShadowMaskDrawer.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Shadow/ShadowDirector.hpp"

namespace al {

/**
 * Initializes the shadow intensity parameters.
 */
void ShadowMaskParam::init() {
    mBlockIntensity.init(180, "BlockIntensity", "ブロックのシャドウ強度", "Min=0, Max=255", this);
    mItemIntensity.init(200, "ItemIntensity", "アイテムのシャドウ強度", "Min=0, Max=255", this);
    mMapObjIntensity.init(180, "MapObjIntensity", "地形オブジェのシャドウ強度", "Min=0, Max=255",
                          this);
    mEnemyIntensity.init(200, "EnemyIntensity", "敵のシャドウ強度", "Min=0, Max=255", this);
    mPlayerIntensity.init(200, "PlayerIntensity", "プレイヤーのシャドウ強度", "Min=0, Max=255",
                          this);
}

/**
 * Compares the shadow intensity parameters.
 * @param rOther Parameters to compare with.
 * @return Whether all parameters are equal.
 */
bool ShadowMaskParam::operator==(const ShadowMaskParam& rOther) const {
    return *mBlockIntensity == *rOther.mBlockIntensity &&
           *mItemIntensity == *rOther.mItemIntensity &&
           *mMapObjIntensity == *rOther.mMapObjIntensity &&
           *mEnemyIntensity == *rOther.mEnemyIntensity &&
           *mPlayerIntensity == *rOther.mPlayerIntensity;
}

/**
 * Copies the shadow intensity parameters.
 * @param rOther Parameters to copy.
 * @return This.
 */
ShadowMaskParam& ShadowMaskParam::operator=(const ShadowMaskParam& rOther) {
    *mBlockIntensity = *rOther.mBlockIntensity;
    *mItemIntensity = *rOther.mItemIntensity;
    *mMapObjIntensity = *rOther.mMapObjIntensity;
    *mEnemyIntensity = *rOther.mEnemyIntensity;
    *mPlayerIntensity = *rOther.mPlayerIntensity;
    return *this;
}

/**
 * Interpolates the shadow intensity parameters.
 * @param rParamA Start parameters.
 * @param rParamB End parameters.
 * @param rate Interpolation rate.
 */
void ShadowMaskParam::interp(const ShadowMaskParam& rParamA, const ShadowMaskParam& rParamB,
                             f32 rate) {
    *mBlockIntensity = lerpValueNew(*rParamA.mBlockIntensity, *rParamB.mBlockIntensity, rate);
    *mItemIntensity = lerpValueNew(*rParamA.mItemIntensity, *rParamB.mItemIntensity, rate);
    *mMapObjIntensity = lerpValueNew(*rParamA.mMapObjIntensity, *rParamB.mMapObjIntensity, rate);
    *mEnemyIntensity = lerpValueNew(*rParamA.mEnemyIntensity, *rParamB.mEnemyIntensity, rate);
    *mPlayerIntensity = lerpValueNew(*rParamA.mPlayerIntensity, *rParamB.mPlayerIntensity, rate);
}

/**
 * Gets the shadow intensity of a draw category.
 * @param category Draw category.
 * @return Shadow intensity.
 */
u8 ShadowMaskKeeper::getShadowIntensity(s32 category) const {
    s32 intensity = 0;

    switch (category) {
    case ShadowMaskDrawCategory::Block:
        intensity = mCurrentParam.getBlockIntensity();
        break;
    case ShadowMaskDrawCategory::Item:
        intensity = mCurrentParam.getItemIntensity();
        break;
    case ShadowMaskDrawCategory::MapObj:
        intensity = mCurrentParam.getMapObjIntensity();
        break;
    case ShadowMaskDrawCategory::Enemy:
        intensity = mCurrentParam.getEnemyIntensity();
        break;
    case ShadowMaskDrawCategory::Player:
    case ShadowMaskDrawCategory::PlayerDecoration:
        intensity = mCurrentParam.getPlayerIntensity();
        break;
    default:
        break;
    }

    return intensity;
}

}  // namespace al

/**
 * Gets the shadow mask keeper of the scene an actor belongs to.
 * @param pActor Actor.
 * @return Shadow mask keeper.
 */
al::ShadowMaskKeeper* ShadowMaskFunction::getShadowMaskKeeper(const al::LiveActor* pActor) {
    return static_cast<al::GraphicsSystemInfo*>(pActor->getSceneInfo()->_78)
        ->mShadowDirector->getShadowMaskKeeper();
}
