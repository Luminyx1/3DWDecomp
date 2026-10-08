#pragma once
#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
}
class ITentackSwingTentacleHolder;
class TentackHead;
class TentackTentacleGroup;

/** @brief State where a head breathes fire while its tentacle group retreats. */
class TentackStateAttackShot : public al::ActorStateBase {
public:
    TentackStateAttackShot(TentackHead* pHead, const al::ActorInitInfo& rInfo,
                           const ITentackSwingTentacleHolder* pSwingHolder);
    void receiveDamage();
    void registerTentacleGroup(TentackTentacleGroup* pGroup);

private:
    unsigned char mUnknown20[0x20];

public:
    f32 _40;  // 0x40
};
static_assert(sizeof(TentackStateAttackShot) == 0x48);
