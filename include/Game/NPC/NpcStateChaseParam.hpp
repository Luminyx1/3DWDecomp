#pragma once

#include <prim/seadSafeString.h>

class NpcStateChaseParam {
public:
    NpcStateChaseParam();
    NpcStateChaseParam(float acceleration, float minFrames, float maxFrames,
                       float turnDegrees, float endFrames, bool isEndless,
                       bool avoidFalls, bool avoidAreas, const char* pChaseAction,
                       const char* pEndAction, float param88);

    float mAcceleration;
    int mMinFrames;
    int mMaxFrames;
    float mTurnDegrees;
    int mEndFrames;
    bool mIsEndless;
    bool mAvoidFalls;
    bool mAvoidAreas;
    sead::FixedSafeString<32> mChaseAction;
    sead::FixedSafeString<32> mEndAction;
    float mParam88;
};
