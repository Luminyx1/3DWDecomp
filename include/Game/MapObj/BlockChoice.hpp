#pragma once
#include "Library/LiveActor/LiveActor.hpp"
#include <math/seadVector.h>
class BlockChoice : public al::LiveActor {
public:
    BlockChoice(const char*);
    void appearItemAll(bool);
    void setDisappear();
    bool isActivated() const { return mIsActivated; }
private:
    bool mIsActivated;
    bool mIsBig;
    int mItemType;
    int mItemCount;
    sead::Vector3f mFront;
};
