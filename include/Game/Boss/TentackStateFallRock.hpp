#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

class TentackHead;

struct TentackStateFallRockParam {
    TentackStateFallRockParam();

    int mValue00;
    int mValue04;
    int mValue08;
    int mValue0C;
    float mValue10;
    float mValue14;
    float mValue18;
    bool mFlag1C;
    bool mFlag1D;
};

class TentackStateFallRock : public al::NerveStateBase {
public:
    TentackStateFallRock(TentackHead* pHead, const TentackStateFallRockParam* pParam);

    TentackHead* mHead;
    const TentackStateFallRockParam* mParam;
    int mValue28;
    bool mFlag2C;
    bool mIsValidFollowForce;
};
