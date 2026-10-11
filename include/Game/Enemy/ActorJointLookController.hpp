#pragma once
#include "math/seadMatrix.h"
namespace al { class LiveActor; }
class ActorJointLookControllerParam {
public:
    ActorJointLookControllerParam(float rate, const sead::Vector2f& rRange, bool flag, const sead::Matrix34f* pJoint, const sead::Matrix34f* pBase);
private:
    float mRate;
    sead::Vector2f mRange;
    bool mFlag;
    const sead::Matrix34f* mpJoint;
    const sead::Matrix34f* mpBase;
};
static_assert(sizeof(ActorJointLookControllerParam) == 0x20);

class ActorJointLookController {
public:
    ActorJointLookController(const al::LiveActor* pActor, int capacity);
    void appendJoint(const char*, const sead::Vector3f&, const ActorJointLookControllerParam*);
    void update();
    void setLookAtNearestPlayer(float distance);
    void clearTarget() { mHasTarget = false; mpTarget = nullptr; }
private:
    const al::LiveActor* mpActor;
    const al::LiveActor* mpTarget;
    unsigned char _010[0x20];
    bool mHasTarget;
    sead::Vector3f mTargetPos;
    sead::Vector2f mRange;
};
static_assert(sizeof(ActorJointLookController) == 0x48);
