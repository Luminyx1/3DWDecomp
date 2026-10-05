#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GeneratorBoxChild;
class GeneratorBox : public al::LiveActor {
public:
    explicit GeneratorBox(const char*);
    GeneratorBoxChild* getChild(int) const;
    bool isBlinkingTime() const;
    int getChildCount() const { return mChildCount; }
    bool isReacting() const { return mReactionTime != 0; }
private:
    u8 mUnreconstructed144[0xa8];
    int mChildCount;
    u8 mUnreconstructed1f0[0xc];
    int mReactionTime;
    u8 mUnreconstructed200[0x40];
};
static_assert(sizeof(GeneratorBox) == 0x240);
