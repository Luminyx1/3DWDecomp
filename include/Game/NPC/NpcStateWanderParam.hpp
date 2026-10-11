#pragma once
#include <prim/seadSafeString.h>

class NpcStateWanderParam {
public:
    NpcStateWanderParam();
    ~NpcStateWanderParam();
    NpcStateWanderParam(int waitFrames, int walkFrames, float acceleration, float turnDegrees,
                        float reachedDistance, float territoryRadius, float durationRandomness,
                        float param1C, float param20, bool avoidFalls, bool avoidAreas,
                        const char* pWalkAction, const char* pWaitAction, bool moveWhileTurning,
                        int targetInterval, float centerResetDistance);
    int mWaitFrames;
    int mWalkFrames;
    float mAcceleration;
    float mTurnDegrees;
    float mReachedDistance;
    float mTerritoryRadius;
    float mDurationRandomness;
    float mParam1C;
    float mParam20;
    bool mAvoidFalls;
    bool mAvoidAreas;
    sead::FixedSafeString<32> mWalkAction;
    sead::FixedSafeString<32> mWaitAction;
    bool mMoveWhileTurning;
    int mTargetInterval;
    float mCenterResetDistance;
};
