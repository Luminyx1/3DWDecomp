#pragma once

#include <math/seadVector.h>

namespace al {
class Rail;

class RailRider {
public:
    RailRider(const Rail*);

    void moveToRailStart();
    void move();
    void syncPosDir();
    void setCoord(f32);
    void moveToRailEnd();
    void moveToBegin();
    void moveToGoal();
    void moveToNearestRail(const sead::Vector3f&);
    void getUpDir(sead::Vector3f*);
    void reverse();
    void setMoveGoingStart();
    void setMoveGoingEnd();
    void setSpeed(f32);
    void addSpeed(f32);
    void scaleSpeed(f32);
    bool isReachedGoal() const;
    bool isReachedRailEnd() const;
    bool isReachedRailStart() const;
    bool isReachedEdge() const;

    const Rail* getRail() const { return mRail; }

    const sead::Vector3f& getPosition() const { return mPosition; }

    const sead::Vector3f& getDirection() const { return mDirection; }

    f32 getCoord() const { return mCoord; }

    f32 getSpeed() const { return mSpeed; }

    bool isMoveForwards() const { return mIsMoveForwards; }

private:
    const Rail* mRail;                                  // _0
    sead::Vector3f mPosition = sead::Vector3f::zero;    // _8
    sead::Vector3f mDirection = sead::Vector3f::zero;   // _14
    f32 mCoord = 0.0f;                                  // _20
    f32 mSpeed = 0.0f;                                  // _24
    bool mIsMoveForwards = true;                        // _28
};

}  // namespace al
