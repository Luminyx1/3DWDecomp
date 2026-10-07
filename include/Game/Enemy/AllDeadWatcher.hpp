#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class AllDeadWatcher : public al::LiveActor {
public:
    AllDeadWatcher(const char*);
    ~AllDeadWatcher() override {}
    void init(const al::ActorInitInfo&) override;
    virtual void exeWatch();
    virtual void exeWait();
    virtual void exeWatchPartial();
    void setWaitNerve();
    void setWatchNerve();
protected:
    al::LiveActor** mActors;
    void* _150;
    int mActorCount;
    unsigned char _15c[0xb];
};
static_assert(sizeof(AllDeadWatcher) == 0x168);
