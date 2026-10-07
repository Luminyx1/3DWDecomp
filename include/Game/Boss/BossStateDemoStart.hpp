#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class CameraInfo;
class LiveActor;
}  // namespace al

class BossDemoStartInfo {
public:
    BossDemoStartInfo(al::CameraInfo* pCameraInfo, al::LiveActor* pActor, const char* pActionName,
                      s32 demoFrame, const sead::Matrix34f* pBaseMtx);

private:
    u8 _0[0x18];

public:
    s32 _18;

private:
    u8 _1C[0xc];
};
static_assert(sizeof(BossDemoStartInfo) == 0x28);

class BossStateDemoStart : public al::ActorStateBase {
public:
    BossStateDemoStart(al::LiveActor* pActor, const al::ActorInitInfo& rInfo,
                       BossDemoStartInfo* pInfo);

private:
    u8 _20[0x24];

public:
    bool _44;

private:
    u8 _45[0x8b];
};
static_assert(sizeof(BossStateDemoStart) == 0xd0);
