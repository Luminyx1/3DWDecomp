#pragma once

#include <math/seadVector.h>

/// The wall the player snapped to (implemented by PlayerCollider).
class IUsePlayerSnapWallInfo {
public:
    virtual bool isSnapWallExist() const = 0;
    virtual const sead::Vector3f& getSnapWallLastNormal() const = 0;
    virtual const sead::Vector3f& getSnapWallNormal() const = 0;
    virtual const sead::Vector3f& getSnapWallPos() const = 0;
};
