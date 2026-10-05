#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GoalItem;
class KinopioBrigadeNpc : public al::LiveActor {
public:
    explicit KinopioBrigadeNpc(const char*);
    GoalItem* getGoalItem();
    bool isDiscovered() const;
    int getMemberIndex() const { return mMemberIndex; }
private:
    u8 mUnknown144[0x50];
    int mMemberIndex;
    u8 mUnknown198[0x70];
};
static_assert(sizeof(KinopioBrigadeNpc) == 0x208);
