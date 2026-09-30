#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;

enum class ActorParamType : s32 {
    None = -1,
    S32 = 0,
    F32 = 1,
    Move = 2,
    Jump = 3,
    Sight = 4,
    Rebound = 5,
};

struct ActorParamS32 {
    s32 value;
};

struct ActorParamF32 {
    f32 value;
};

struct ActorParamMove {
    f32 moveAccel;
    f32 gravity;
    f32 moveFriction;
    f32 turnSpeedDegree;
};

struct ActorParamJump {
    f32 speedFront;
    f32 speedUp;
};

struct ActorParamSight {
    f32 distance;
    f32 degreeH;
    f32 degreeV;
};

struct ActorParamRebound {
    f32 reboundRate;
    f32 speedMinToRebound;
    f32 frictionH;
};

struct ActorParamInfo {
    ActorParamInfo();

    const char* mName = nullptr;
    ActorParamType mType = ActorParamType::None;

    union {
        ActorParamS32 mParamS32 = {0};
        ActorParamF32 mParamF32;
        ActorParamMove* mParamMove;
        ActorParamJump* mParamJump;
        ActorParamSight* mParamSight;
        ActorParamRebound* mParamRebound;
    };
};

class ActorParamHolder {
public:
    static ActorParamHolder* tryCreate(LiveActor* pActor);

    ActorParamHolder(LiveActor* pActor);

    const ActorParamS32* findParamS32(const char* pName) const;
    ActorParamInfo* tryFindParamInfoByName(const char* pName) const;
    const ActorParamF32* findParamF32(const char* pName) const;
    const ActorParamMove* findParamMove(const char* pName) const;
    const ActorParamJump* findParamJump(const char* pName) const;
    const ActorParamSight* findParamSight(const char* pName) const;
    const ActorParamRebound* findParamRebound(const char* pName) const;

    s32 mSize = 0;
    ActorParamInfo* mInfoArray = nullptr;
};
}  // namespace al
