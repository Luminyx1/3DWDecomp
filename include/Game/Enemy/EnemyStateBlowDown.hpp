#pragma once

#include "Library/Nerve/NerveStateBase.hpp"
#include <math/seadVector.h>

namespace al {
class HitSensor;
}

struct EnemyStateBlowDownParam {
    EnemyStateBlowDownParam(bool isSmall);
    EnemyStateBlowDownParam(const char* pAction);
    EnemyStateBlowDownParam(const char* pAction, float jumpSpeed, float speed,
                           float friction, float gravity, int step);

    const char* mAction;
    float mJumpSpeed;
    float mSpeed;
    float mFriction;
    float mGravity;
    int mStep;
};

class EnemyStateBlowDown : public al::ActorStateBase {
public:
    EnemyStateBlowDown(const char* pName, al::LiveActor* pHost,
                       const EnemyStateBlowDownParam* pParam);
    EnemyStateBlowDown(al::LiveActor* pHost, const EnemyStateBlowDownParam* pParam);
    ~EnemyStateBlowDown() override = default;
    void appear() override;
    void kill() override;
    void setBlowDir(const al::LiveActor* pActor);
    void setBlowDir(const sead::Vector3f& rDir);
    void setBlowDir(const al::HitSensor* pOther, const al::HitSensor* pSelf);
    void exeDown();

private:
    const EnemyStateBlowDownParam* mParam;
    sead::Vector3f mBlowDir = sead::Vector3f::ez;
    bool mIsHit = false;
};

static_assert(sizeof(EnemyStateBlowDownParam) == 0x20);
static_assert(sizeof(EnemyStateBlowDown) == 0x38);
