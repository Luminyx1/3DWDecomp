#pragma once

#include <basis/seadTypes.h>

/// How charged a squat is, 0 to 1 (e.g. PlayerActionSquatWalk).
class IUsePlayerSquatEnergy {
public:
    virtual f32 getEnergy() const = 0;
};
