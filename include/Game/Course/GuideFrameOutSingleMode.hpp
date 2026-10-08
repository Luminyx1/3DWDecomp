#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
class LiveActor;
}  // namespace al

/**
 * @brief Bowser's Fury HUD marker pointing at an actor (Bowser Jr., Fury Bowser) that is off screen.
 * @note Only what reconstructed code needs is declared so far.
 */
class GuideFrameOutSingleMode : public al::LayoutActor {
public:
    GuideFrameOutSingleMode(const al::LayoutInitInfo& rInfo, const al::LiveActor* pTarget,
                            const char* pName);

    void appear() override;
    void control() override;
    void startDemo();
    void updateVisibility(bool isVisible);
    void endDemo();
    void startPause();
    void endPause();
    void setEnable();
    void setDisable();
    void setEnableBubble(bool isEnable);
    void setEnableWarpGuide(bool isEnable);
    bool isEnable() const;
    bool isScreenOut() const;
    s32 getScreenOutFrame() const;
    void updateWarpGuide(f32 rate);
    void disappearImmediate();
    void initIcon();

    void exeStartAppear();
    void exeAppear();
    void exeStartDisappear();
    void exeDisappear();

    /**
     * @brief Set the unidentified float at 0x148.
     * @param value The new value.
     */
    void setUnk148(f32 value) { _148 = value; }

private:
    u8 _121[0x148 - 0x121];
    f32 _148;
};

static_assert(sizeof(GuideFrameOutSingleMode) == 0x150);
