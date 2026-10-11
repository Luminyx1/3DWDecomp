#pragma once

#include "Library/Nerve/NerveExecutor.hpp"
#include <math/seadVector.h>

namespace al { class LiveActor; }
class RabbitRoute;
class IUseRabbitRoutePoint;

class RabbitRouteRider : public al::NerveExecutor {
public:
    RabbitRouteRider(const al::LiveActor* pActor, const RabbitRoute* pRoute, bool isFastJump);
    /** @brief Releases the rider's nerve executor. */
    ~RabbitRouteRider() override = default;
    void updatePoint(const IUseRabbitRoutePoint* pPoint, const IUseRabbitRoutePoint* pNextPoint);
    void move();
    bool isJumping() const;
    bool isJumpStart() const;
    const sead::Vector3f& getCurrentPointPos() const;
    bool isNearNextPoint() const;
    const sead::Vector3f& getNextPointPos() const;
    void reverse();
    void updateNextPointReversePlayer(const IUseRabbitRoutePoint* pPreviousPoint);
    void updateRotate();
    /** @brief Gets the route direction. @return Current movement direction. */
    const sead::Vector3f& getDirection() const { return mDirection; }
    void setSpeed(float speed);
    bool tryStartJump();
    void exeMove();
    void exeJump();

private:
    const al::LiveActor* mActor;
    bool mIsFastJump;
    const IUseRabbitRoutePoint* mCurrentPoint;
    const IUseRabbitRoutePoint* mNextPoint;
    sead::Vector3f mPosition;
    sead::Vector3f mDirection;
    float mSpeed;
    int mJumpSteps;
    int mJumpHeight;
};
