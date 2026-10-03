#pragma once

#include <basis/seadTypes.h>

namespace al {
class SeRequest;
class SeRequestKeeper;

class SeWaitingListKeeper {
  public:
    SeWaitingListKeeper();
    void update(SeRequestKeeper* pKeeper);
    void addSe(SeRequest* pRequest);

  private:
    struct Entry {
        SeRequest* mRequest = nullptr;
        s32 mFrames = -1;
    };

    Entry** mEntries;
    s32 mCapacity = 77;
};

static_assert(sizeof(SeWaitingListKeeper) == 0x10);
} // namespace al
