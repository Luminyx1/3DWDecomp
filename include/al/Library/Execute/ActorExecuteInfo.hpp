#pragma once

#include <basis/seadTypes.h>

namespace al {
class ExecuteRequestKeeper;
class ExecutorActorExecuteBase;
class ModelDrawerBase;

class ActorExecuteInfo {
public:
    ActorExecuteInfo(ExecuteRequestKeeper* pKeeper);

    void addUpdater(ExecutorActorExecuteBase* pUpdater);
    void addDrawer(ModelDrawerBase* pDrawer);
    void removeDrawer(ModelDrawerBase* pDrawer);
    ModelDrawerBase* removeOptDrawer();

    ExecuteRequestKeeper* mRequestKeeper;
    s32 mUpdaterCount = 0;
    ExecutorActorExecuteBase* mUpdaters[4] = {};
    s32 mDrawerCount = 0;
    ModelDrawerBase* mDrawers[5] = {};
};

static_assert(sizeof(ActorExecuteInfo) == 0x60);

struct ExecuteRequestInfo {
    ExecuteRequestInfo();

    void* _0;
};
}  // namespace al
