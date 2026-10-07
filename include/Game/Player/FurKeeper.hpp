#pragma once

namespace al {
class LiveActor;
}

class FurKeeper {
public:
    FurKeeper();

    void init(al::LiveActor* pActor, const char* pName);
    void updateUbo();
    void draw() const;

private:
    unsigned char _0[0x30];
};

static_assert(sizeof(FurKeeper) == 0x30);
