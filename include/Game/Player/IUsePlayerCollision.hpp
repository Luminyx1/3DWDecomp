#pragma once

#include <math/seadVector.h>

/// Player collision queries and solving (implemented by PlayerCollider).
class IUsePlayerCollision {
public:
    struct Info;

    virtual void clear() = 0;
    virtual void moveSimple(bool) = 0;
    virtual void solveAir() = 0;
    virtual void solveAirNoFloor() = 0;
    virtual void snapGround() = 0;
    virtual void snapWall(bool) = 0;
    virtual void push(const sead::Vector3f&) = 0;
    virtual void clearPush() = 0;
    virtual void arrangeJumpFollowVel() = 0;
    virtual void clearJumpFollowVel() = 0;
    virtual bool isOnFloor() const = 0;
    virtual bool isOnFrontWall() const = 0;
    virtual bool isOnBackWall() const = 0;
    virtual bool isOnCeiling() const = 0;
    virtual bool isOnRightWall() const = 0;
    virtual bool isOnLeftWall() const = 0;
    virtual bool isOnAnyWall() const = 0;
    virtual bool isHeadOnFrontWall() const = 0;
    virtual bool isCenterOnFloor() const = 0;
    virtual void setDisableLegCheck(bool) = 0;
    virtual void setDisableLegCheckForce(bool) = 0;
    virtual bool getFloorInfo(Info*) const = 0;
    virtual bool getFrontWallInfo(Info*) const = 0;
    virtual bool getBackWallInfo(Info*) const = 0;
    virtual bool getCeilingInfo(Info*) const = 0;
    virtual bool getRightWallInfo(Info*) const = 0;
    virtual bool getLeftWallInfo(Info*) const = 0;
};
