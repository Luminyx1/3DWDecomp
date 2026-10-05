#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class KarakuriCastleDoor : public al::LiveActor {
public:
    KarakuriCastleDoor(const char*);
    void setParam(int, int, float);
    void startOpen(const sead::Vector3f&, int);
    bool isOpenReady() const;
    bool isOpen() const;
    bool isOpening() const;
    int getIndex() const { return mIndex; }
private:
    int mIndex;
    unsigned char _148[0x30];
};
