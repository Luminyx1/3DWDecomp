#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;

    struct ActorParamS32 {
        s32 mValue;
    };

    struct ActorParamF32 {
        f32 mValue;
    };

    struct ActorParamMove {
        f32 mMoveAccel;         // _0
        f32 mGravity;           // _4
        f32 mMoveFriction;      // _8
        f32 mTurnSpeedDegree;   // _C
    };

    struct ActorParamJump {
        f32 mSpeedFront;        // _0
        f32 mSpeedUp;           // _4
    };

    struct ActorParamSight {
        f32 mDistance;          // _0
        f32 mDegreeH;           // _4
        f32 mDegreeV;           // _8
    };

    struct ActorParamRebound {
        f32 mReboundRate;       // _0
        f32 mSpeedMinToRebound; // _4
        f32 mFrictionH;         // _8
    };

    /// One named parameter read from an actor's ActorParam resource.
    struct ActorParamInfo {
        enum Type {
            Type_None = -1,
            Type_S32 = 0,
            Type_F32 = 1,
            Type_Move = 2,
            Type_Jump = 3,
            Type_Sight = 4,
            Type_Rebound = 5
        };

        __attribute__((used)) ActorParamInfo() : mName(nullptr), mType(Type_None) { mS32.mValue = 0; }

        const char* mName;                  // _0
        s32 mType;                          // _8
        union {
            ActorParamS32 mS32;
            ActorParamF32 mF32;
            ActorParamMove* mMove;
            ActorParamJump* mJump;
            ActorParamSight* mSight;
            ActorParamRebound* mRebound;
        };                                  // _10
    };

    /// Named parameters read from an actor's ActorParam resource file.
    class ActorParamHolder {
    public:
        static ActorParamHolder* tryCreate(LiveActor* pActor);

        ActorParamHolder(LiveActor* pActor);

        const ActorParamS32* findParamS32(const char* pName) const;
        const ActorParamF32* findParamF32(const char* pName) const;
        const ActorParamMove* findParamMove(const char* pName) const;
        const ActorParamJump* findParamJump(const char* pName) const;
        const ActorParamSight* findParamSight(const char* pName) const;
        const ActorParamRebound* findParamRebound(const char* pName) const;
        ActorParamInfo* tryFindParamInfoByName(const char* pName) const;

        s32 mNumParams;                 // _0
        ActorParamInfo* mParams;        // _8
    };

    const ActorParamF32* findActorParamF32(const LiveActor* pActor, const char* pName);
    const ActorParamS32* findActorParamS32(const LiveActor* pActor, const char* pName);
    const ActorParamMove* findActorParamMove(const LiveActor* pActor, const char* pName);
    const ActorParamJump* findActorParamJump(const LiveActor* pActor, const char* pName);
    const ActorParamSight* findActorParamSight(const LiveActor* pActor, const char* pName);
    const ActorParamRebound* findActorParamRebound(const LiveActor* pActor, const char* pName);
    void setActorParamMove(ActorParamMove* pParam, f32 moveAccel, f32 gravity, f32 moveFriction, f32 turnSpeedDegree);
};
