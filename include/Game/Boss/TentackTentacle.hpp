#pragma once
#include "Library/LiveActor/LiveActor.hpp"

class TentackBase;

/** @brief Per-attack swing setup of a tentacle. */
class TentackTentacleInfo {
public:
    void reset();

private:
    unsigned char mUnknown[0x124];
};
static_assert(sizeof(TentackTentacleInfo) == 0x124);

/** @brief Small snake tentacle of the Tentack boss. */
class TentackTentacle : public al::LiveActor {
public:
    TentackTentacle(const char* pName, TentackBase* pHost);
    void receiveDamage(bool isLast, s32 damage);
    bool isAppear() const;
    bool isDamage() const;
    void eatAttachItemIfAttached(bool isForce);
    void endSwingForce();
    void setSwingYRate(f32 rate);

    /** @brief Gets the swing setup of the current attack. @return Tentacle info. */
    TentackTentacleInfo* getInfo() { return &mInfo; }

    /** @brief Checks whether the tentacle is a spare that does not attack. @return True if spare. */
    bool isSpare() const { return mIsSpare; }

    TentackBase* mHost;         // 0x148
    TentackTentacleInfo mInfo;  // 0x150
    bool mIsSpare;              // 0x274
private:
    unsigned char mUnknown275[0x13];
};
static_assert(sizeof(TentackTentacle) == 0x288);
