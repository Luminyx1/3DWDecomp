#pragma once

#include "Library/Scene/ISceneObj.hpp"

/**
 * @brief Scene object tracking the goal items (Cat Shines) of Bowser's Fury.
 * @note Only the members used by reconstructed code are declared.
 */
class GoalItemHolder : public al::ISceneObj {
  public:
    bool isLastShineNeko() const;
    bool isLastShineDisaster() const;
};
