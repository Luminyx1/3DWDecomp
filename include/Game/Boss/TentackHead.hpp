#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class TentackBase;

class TentackHead : public al::LiveActor {
public:
    TentackHead(const char* pName, TentackBase* pHost, bool isSubHead);
    bool tryStartActionAttackRockIfWait();

    TentackBase* mHost;
private:
    // Remaining head state, from offset 0x150 through its verified 0x1e8 allocation.
    unsigned char mUnknown150[0x98];
};
static_assert(sizeof(TentackHead) == 0x1e8);
