#pragma once

#include "Library/Nerve/NerveStateBase.hpp"
#include <math/seadVector.h>

namespace al {
struct ActorInitInfo;
class HitSensor;
class SensorMsg;
class ScreenPointer;
class ScreenPointTarget;
class RumbleCalculatorCosMultLinear;
}
class ActorStateSupportStroke;

class NpcStateWaitParam {
public:
    NpcStateWaitParam(const char* pWaitAction, const char* pWaitAfterAction,
                      const char* pTurnAction, const char* pReactionAction,
                      const char* pMicReactionAction, const char* pStrokeAction,
                      const char* pTrampledAction, bool appearItem,
                      const sead::Vector3f* pItemOffset, bool restrictReaction);
    const char* mWaitAction;
    const char* mWaitAfterAction;
    const char* mTurnAction;
    const char* mReactionAction;
    const char* mMicReactionAction;
    const char* mStrokeAction;
    const char* mTrampledAction;
    bool mAppearItem;
    sead::Vector3f mItemOffset;
    bool mRestrictReaction;
};

class NpcStateTurnParam {
public:
    NpcStateTurnParam(float startAngle, float endAngle, float turnDegrees,
                      float searchRadius, bool isEnabled, bool onlyAfter, int endFrames);
    float mStartAngle;
    float mEndAngle;
    float mTurnDegrees;
    float mSearchRadius;
    bool mIsEnabled;
    bool mOnlyAfter;
    int mEndFrames;
};

class NpcStateRumbleParam {
public:
    NpcStateRumbleParam(int frames, float frequency, float angleOffset, float amplitude, float scale);
    int mFrames;
    float mFrequency;
    float mAngleOffset;
    float mAmplitude;
    float mScale;
};

class NpcStateWait : public al::ActorStateBase {
public:
    NpcStateWait(al::LiveActor* pActor, const al::ActorInitInfo& rInitInfo,
                 const NpcStateWaitParam* pWaitParam, const NpcStateTurnParam* pTurnParam,
                 const NpcStateRumbleParam* pRumbleParam);
    ~NpcStateWait() override;
    void appear() override;
    void control() override;
    void setWaitAfter();
    void setWait();
    void startWait();
    void invalidateTurn();
    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther);
    bool receiveMsg(const al::SensorMsg* pMessage, al::HitSensor* pOther, al::HitSensor* pSelf);
    bool tryStartReaction();
    bool receiveMsgScreenPoint(const al::SensorMsg* pMessage, al::ScreenPointer* pPointer,
                               al::ScreenPointTarget* pTarget);
    void startTurnEnd();
    bool tryStartTurn(const NpcStateTurnParam* pParam);
    bool tryStartMicReaction();
    void exeWait();
    void exeStroke();
    void exeWaitAfter();
    void exeTurn();
    void exeTrampled();
    void exeReaction();
    void exeMicReaction();
    void exeTurnEnd();

private:
    ActorStateSupportStroke* mStrokeState;
    const NpcStateWaitParam* mWaitParam;
    const NpcStateTurnParam* mTurnParam;
    const NpcStateRumbleParam* mRumbleParam;
    al::RumbleCalculatorCosMultLinear* mRumbleCalculator;
    int mRumbleFrame;
    int mReactionCooldown;
    bool mIsWaitAfter;
    bool mIsTurnDisabled;
};
