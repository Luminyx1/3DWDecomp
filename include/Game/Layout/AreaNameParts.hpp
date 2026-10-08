#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
class PlayerHolder;
class WipeSimple;
}  // namespace al

class ScenarioInfo;

/**
 * @brief Bowser's Fury HUD banner showing the name of the island / scenario the player enters.
 * @note Only what reconstructed code needs is declared so far.
 */
class AreaNameParts : public al::LayoutActor {
public:
    AreaNameParts(const al::LayoutInitInfo& rInfo, const char* pName, const char* pPartsName,
                  al::LayoutActor* pParent, const al::PlayerHolder* pPlayerHolder);

    void changeName(const ScenarioInfo& rInfo);
    bool isFadeOut();
    void updateTextBoxes();
    void fadeOut(bool isForce);
    void forceEndAppear(bool isPhaseStart);
    void setWipeFadeWhite(al::WipeSimple* pWipe);
    void handleIslandWarp();
    void endDemo(bool isAppear);

    void exeAppear();
    void exeWait();
    void exeWaitFadeWhite();
    void exeChangeName();
    void exeFadeOut();

private:
    u8 _121[0x160 - 0x121];
};

static_assert(sizeof(AreaNameParts) == 0x160);
