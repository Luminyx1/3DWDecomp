#pragma once
#include <basis/seadTypes.h>
#include <thread/seadEvent.h>
namespace rc {
class AssetLoadingThread {
  public:
    enum LOAD_TYPE : unsigned int;
    bool isLoading(LOAD_TYPE type);
    bool isLoadDone(LOAD_TYPE type);
    bool isLoadOrLoading(LOAD_TYPE type);
    void disableFastLoad(bool disable);

  private:
    u8 mUnreconstructed00[0x48]; // Thread, heap, delegate, and cancellation state.
    sead::Event* mpDoneEvents;
    sead::Event* mpLoadingEvents;
    void* mpUnknown58;
    bool mFastLoad;
};
static_assert(sizeof(AssetLoadingThread) == 0x68);
} // namespace rc
