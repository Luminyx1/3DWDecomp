#pragma once

#include <prim/seadRuntimeTypeInfo.h>

class PlayerAction {
    SEAD_RTTI_BASE(PlayerAction)

public:
    virtual ~PlayerAction() = default;
    virtual void move() = 0;
    virtual void update() = 0;
    virtual void setup() = 0;
    virtual void teardown() = 0;
};
