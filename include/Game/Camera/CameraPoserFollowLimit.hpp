#pragma once

#include <basis/seadTypes.h>

#include "Library/Camera/CameraPoser_RS.hpp"

/**
 * @brief Follow camera poser whose movement is limited by the stage.
 * @note Only what reconstructed code needs is declared so far.
 */
class CameraPoserFollowLimit : public al::CameraPoser_RS {
public:
    void setTargetAngleV(f32 angle);
};
