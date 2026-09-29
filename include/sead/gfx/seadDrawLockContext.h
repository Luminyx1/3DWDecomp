#pragma once

#include "hostio/seadHostIONode.h"
#include "thread/seadCriticalSection.h"

namespace sead
{
class DrawLockContext : public hostio::Node
{
public:
    DrawLockContext();

    void initialize(Heap* heap);
    void lock();
    void unlock();
    void genMessage(hostio::Context* context);

private:
    [[maybe_unused]] u32 _8 = 0;
    CriticalSection mCriticalSection{};
    u8 _50[0x10];
};

}  // namespace sead
