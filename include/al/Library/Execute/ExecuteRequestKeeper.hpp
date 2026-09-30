#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;

class ExecuteRequestTable {
public:
    ExecuteRequestTable(s32 maxSize);

    void addRequest(LiveActor* pActor) {
        for (s32 i = 0; i < mSize; i++) {
            if (mRequests[i] == pActor) {
                return;
            }
        }
        mRequests[mSize++] = pActor;
    }

    void removeRequest(LiveActor* pActor) {
        for (s32 i = 0; i < mSize; i++) {
            if (mRequests[i] == pActor) {
                mRequests[i] = mRequests[mSize - 1];
                mSize--;
            }
        }
    }

    s32 mMaxSize = 0;
    s32 mSize = 0;
    LiveActor** mRequests = nullptr;
};

static_assert(sizeof(ExecuteRequestTable) == 0x10);

class ExecuteRequestKeeper {
public:
    enum Request : s32 {
        Request_Movement = 0,
        Request_RemoveFromMovement = 1,
        Request_Draw = 2,
        Request_RemoveFromDraw = 3,
        Request_DrawImmediate = 4,
    };

    ExecuteRequestKeeper(s32 maxSize);

    void executeRequestActorMovementAllOn();
    void executeRequestActorMovementAllOff();
    void executeRequestActorDrawAllOn();
    void executeRequestActorDrawAllOff();
    void executeRequestActorDrawAllOnImmediate();
    void request(LiveActor* pActor, s32 requestType);

    ExecuteRequestTable* mRequestTables[5];
};

static_assert(sizeof(ExecuteRequestKeeper) == 0x28);
}  // namespace al
