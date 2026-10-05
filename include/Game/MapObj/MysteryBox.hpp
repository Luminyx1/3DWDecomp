#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class MysteryBox : public al::LiveActor {
public:
    explicit MysteryBox(const char*);
    void setDestMysteryBox(const MysteryBox*);
    bool isCountDownEnd() const;
    bool tryCancelCountDown(bool);
    void setAutoCountDownCancel(bool enabled) { mAutoCountDownCancel = enabled; }
private:
    u8 mUnreconstructed144[0x44];
    bool mAutoCountDownCancel;
    u8 mUnreconstructed189[0x37];
};
static_assert(sizeof(MysteryBox) == 0x1c0);
