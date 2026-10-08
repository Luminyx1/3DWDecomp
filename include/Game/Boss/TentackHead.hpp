#pragma once
#include <math/seadVector.h>
#include "Library/LiveActor/LiveActor.hpp"
class TentackBase;
class TentackRockFaller;

class TentackHead : public al::LiveActor {
public:
    TentackHead(const char* pName, TentackBase* pHost, bool isSubHead);
    bool tryStartActionAttackRockIfWait();
    bool tryStartActionEat();
    bool tryStartActionEatAndDisappear();
    bool tryStartActionCry();
    void startActionAttackTentacle();
    void cancelDemoAppear();
    void setWait();
    void setWaitFixed();
    void setDisappear();
    void changeLookTarget();
    bool turnToTargetGently(const sead::Vector3f& rTarget, f32 speed);
    bool turnToDirectionGently(const sead::Vector3f& rDir, f32 speed);
    bool isDemoEnd() const;
    bool isWaitAll() const;
    bool isWaitFixed() const;
    bool isDamageAction() const;
    bool isBackOrPush() const;

    TentackBase* mHost;
private:
    unsigned char mUnknown150[0x8];
public:
    TentackRockFaller* mRockFaller;  // 0x158
    s32 mDamage;                     // 0x160 number of hits taken
    sead::Vector3f mFrontDir;        // 0x164 initial facing direction
private:
    // Remaining head state, from offset 0x170 through its verified 0x1e8 allocation.
    unsigned char mUnknown170[0x78];
};
static_assert(sizeof(TentackHead) == 0x1e8);
