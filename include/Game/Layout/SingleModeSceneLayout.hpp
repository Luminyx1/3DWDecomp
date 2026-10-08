#pragma once

#include "Library/Layout/LayoutActor.hpp"
#include "Library/Scene/ISceneObj.hpp"
#include "Layout/SceneLayoutBase.hpp"

namespace al {
class HitSensor;
class LiveActor;
class WipeSimple;
}  // namespace al

/**
 * @brief The HUD layout of Bowser's Fury.
 * @note Only what reconstructed code needs is declared so far.
 */
class SingleModeSceneLayout : public al::LayoutActor, public SceneLayoutBase, public al::ISceneObj {
public:
    void forceHideShineCounter();
    void setDisableAreaName(bool isDisable);
    void spawnItemStockCoins(u8 count, al::HitSensor* pSensor);
    static SingleModeSceneLayout* tryGetSingleModeSceneLayout(const al::IUseSceneObjHolder* pHolder);
    bool isTimerActive();
    void startDemo(bool isHideAll, bool isInstant);

    /**
     * @brief Get the full-screen wipe used to hide cutscene skips.
     * @return The wipe.
     */
    al::WipeSimple* getWipe() const { return mWipe; }

    /**
     * @brief Get the number of frames the wipe takes to close or open.
     * @return The wipe duration in frames.
     */
    s32 getWipeFrames() const { return mWipeFrames; }

private:
    u8 _138[0x170 - 0x138];
    al::WipeSimple* mWipe;  // 0x170
    s32 mWipeFrames;        // 0x178
};
