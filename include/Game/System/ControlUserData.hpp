#pragma once

#include <basis/seadTypes.h>

class ControlUserData {
  public:
    ControlUserData();
    bool isActive() const;
    bool isDeadInStage() const;
    void resetDeadInStage();
    bool isDeactive() const;
    void setDeadInStage();

    enum State : u32 { Alive = 0, DeadInStage = 1, Deactive = 2 };

    s32 mUserIndex;
    s32 mPadPort = -1;
    State mState = Deactive;
    s32 mCharacterType = -1;
    s32 mSavedCharacterType = -1;
    s32 mFigureType;
};

static_assert(sizeof(ControlUserData) == 0x18);
