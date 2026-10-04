#pragma once

#include "Scene/SingleModeScene.hpp"

/**
 * @brief Scene of a Bowser's Fury exploration phase.
 */
class PhaseScene : public SingleModeScene {
  public:
    PhaseScene();

  private:
    u8 mUnknown378[8]; // Unreconstructed phase state.
};
static_assert(sizeof(PhaseScene) == 0x380);
