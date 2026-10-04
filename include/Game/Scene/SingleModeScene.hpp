#pragma once

#include <basis/seadTypes.h>

/**
 * @brief Base scene of Bowser's Fury.
 * @note The scene state is not reconstructed yet and kept as padding.
 */
class SingleModeScene {
  public:
    explicit SingleModeScene(const char* pName);

  private:
    u8 mUnknown0[0x378]; // Unreconstructed al::Scene and single-mode scene state.
};
