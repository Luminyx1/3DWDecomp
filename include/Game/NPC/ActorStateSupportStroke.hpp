#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class AnimScaleController;
class SensorMsg;
class ScreenPointer;
class ScreenPointTarget;
}

class ActorStateSupportStroke : public al::ActorStateBase {
public:
    explicit ActorStateSupportStroke(al::LiveActor* pActor);
    ~ActorStateSupportStroke() override;
    void kill() override;
    void stopAndReset();
    bool receiveMsgScreenPoint(const al::SensorMsg* pMessage, al::ScreenPointer* pPointer,
                               al::ScreenPointTarget* pTarget);
    void updateScaleAnim();
    void updateAppearItemTiming();
    void updateStrokeTime();
    bool isTouch() const;
    /** @brief Reports the item trigger for this frame. @return Whether an item should appear. */
    bool shouldAppearItem() const { return mShouldAppearItem; }
    void exeWait();
    void exeTouch();

private:
    al::LiveActor* mTouchActor;
    al::AnimScaleController* mScaleController;
    int mTouchFrames;
    int mStrokeGraceFrames;
    int mStrokeFrames;
    bool mHasAppearedItem;
    bool mShouldAppearItem;
    bool mIsScaleEnabled;
    bool mIsEffectEnabled;
};
