#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class PanelNote : public al::LiveActor {
public:
    PanelNote(const char*);
    bool isOn() const;
    void setGroupControlled(bool value) { mIsGroupControlled = value; }
private:
    u8 mUnreconstructed144[0x168 - 0x144];
    bool mIsGroupControlled;
    u8 mUnreconstructed169[0x178 - 0x169];
};
