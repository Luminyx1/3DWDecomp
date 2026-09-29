#pragma once

/// A test an action node runs to decide whether to switch to another action.
class PlayerActionCondition {
public:
    virtual ~PlayerActionCondition() = default;
    virtual bool check() = 0;
    virtual void setup() {}
};
