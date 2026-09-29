#pragma once

#include <math/seadVector.h>

/// The water current pushing the player (implemented by PlayerWaterFlowField).
class IUsePlayerWaterFlowField {
public:
    virtual const sead::Vector3f& getFlowField() const = 0;
};
