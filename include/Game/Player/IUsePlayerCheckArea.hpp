#pragma once

#include <math/seadVector.h>

/// Area queries at a position (implemented by PlayerAreaChecker).
class IUsePlayerCheckArea {
public:
    virtual bool isInAbyss(const sead::Vector3f&) const = 0;
    virtual bool isInWater(const sead::Vector3f&) const = 0;
    virtual bool isInWaterNoSink(const sead::Vector3f&) const = 0;
    virtual bool isInWaterFall(const sead::Vector3f&) const = 0;
    virtual bool isInSinkSandArea(const sead::Vector3f&) const = 0;
    virtual bool isInForceFallArea(const sead::Vector3f&) const = 0;
    virtual bool isInBattleArea(const sead::Vector3f&) const = 0;
    virtual bool isInRestrictedPlaneArea(const sead::Vector3f&) const = 0;
};
