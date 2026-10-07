#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace alSeFunction { enum DemoType : s32; }
// Partial declaration of the inherited cutscene interface and actor layout.
class DemoAnimatic : public al::LiveActor {
public:
    explicit DemoAnimatic(const char*, alSeFunction::DemoType = static_cast<alSeFunction::DemoType>(5));
    virtual void setBgmRequest(const char*, unsigned int, al::AudioKeeper*);
    virtual void startDemo();
    virtual void endDemo(bool);
    virtual void setEndSceneFlag();
private:
    u8 mUnreconstructed[0x224];
};
static_assert(sizeof(DemoAnimatic) == 0x368);
