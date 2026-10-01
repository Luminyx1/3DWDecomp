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

    ExecuteRequestKeeper* getRequestKeeper() const { return mRequestKeeper; }
    s32 getUpdaterCount() const { return mUpdaterCount; }
    ExecutorActorExecuteBase* getUpdater(s32 index) const { return mUpdaters[index]; }
    s32 getDrawerCount() const { return mDrawerCount; }
    ModelDrawerBase* getDrawer(s32 index) const { return mDrawers[index]; }

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
