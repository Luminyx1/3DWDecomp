#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class CollisionParts;
}

/// Casts a line segment against the map (implemented by PlayerCollisionCheckArrow).
class IUsePlayerCollisionCheckArrow {
public:
    virtual bool checkArrow(const sead::Vector3f&, const sead::Vector3f&) = 0;
    virtual u32 getNum() const = 0;
    virtual const sead::Vector3f& getPos(u32) const = 0;
    virtual const sead::Vector3f& getNormal(u32) const = 0;
    virtual const al::CollisionParts* getCollisionParts(u32) const = 0;
    virtual const char* getMapCodeName(u32) const = 0;
    virtual const char* getWallCodeName(u32) const = 0;
    virtual const char* getMaterialCodeName(u32) const = 0;
};
