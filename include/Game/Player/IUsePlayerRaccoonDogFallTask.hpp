#pragma once

/// The tanooki slow fall (implemented by PlayerRaccoonDogFallTask).
class IUsePlayerRaccoonDogFallTask {
public:
    virtual void setup() = 0;
    virtual bool isFirstFalling() const = 0;
    virtual bool isPossibleToDampVelocity() const = 0;
    virtual void validateDamp() = 0;
    virtual void invalidateDamp() = 0;
};
