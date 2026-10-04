#pragma once

#include "Scene/SingleModeScene.hpp"

/**
 * @brief Scene of a Bowser's Fury boss phase.
 */
class PhaseBossScene : public SingleModeScene {
  public:
    PhaseBossScene();
    explicit PhaseBossScene(const char* pName);

  private:
    u8 mUnknown378[8]; // Unreconstructed boss-phase state.
};
static_assert(sizeof(PhaseBossScene) == 0x380);
