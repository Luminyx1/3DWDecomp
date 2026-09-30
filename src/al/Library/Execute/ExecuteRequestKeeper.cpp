#include "Library/Execute/ExecuteRequestKeeper.hpp"

#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/Execute/ExecutorActorExecuteBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelDrawerBase.hpp"
#include "Library/Model/ModelKeeper.hpp"

namespace al {
/**
 * Allocates an empty request table.
 * @param maxSize Maximum number of requests.
 */
ExecuteRequestTable::ExecuteRequestTable(s32 maxSize) : mMaxSize(maxSize) {
    LiveActor** actors = new LiveActor*[mMaxSize];
    for (s64 i = 0; i != mMaxSize; i++) {
        actors[i] = nullptr;
    }

    mRequests = actors;
}

/**
 * Allocates the request tables.
 * @param maxSize Maximum number of requests per table.
 */
ExecuteRequestKeeper::ExecuteRequestKeeper(s32 maxSize) {
    for (s32 i = 0; i < 5; i++) {
        mRequestTables[i] = new ExecuteRequestTable(maxSize);
    }
}

/**
 * Adds all actors requested for movement to their updaters.
 */
void ExecuteRequestKeeper::executeRequestActorMovementAllOn() {
    ExecuteRequestTable* table = mRequestTables[Request_Movement];
    for (s32 i = 0; i < table->mSize; i++) {
        LiveActor* actor = table->mRequests[i];
        ActorExecuteInfo* info = actor->mActorExecuteInfo;
        for (s32 j = 0; j < info->mUpdaterCount; j++) {
            info->mUpdaters[j]->addActor(actor);
        }
    }

    table->mSize = 0;
}

/**
 * Removes all actors requested for movement removal from their updaters.
 */
void ExecuteRequestKeeper::executeRequestActorMovementAllOff() {
    ExecuteRequestTable* table = mRequestTables[Request_RemoveFromMovement];
    for (s32 i = 0; i < table->mSize; i++) {
        LiveActor* actor = table->mRequests[i];
        ActorExecuteInfo* info = actor->mActorExecuteInfo;
        for (s32 j = 0; j < info->mUpdaterCount; j++) {
            info->mUpdaters[j]->removeActor(actor);
        }
    }

    table->mSize = 0;
}

/**
 * Adds the models of all actors requested for drawing to their drawers.
 */
void ExecuteRequestKeeper::executeRequestActorDrawAllOn() {
    ExecuteRequestTable* table = mRequestTables[Request_Draw];
    for (s32 i = 0; i < table->mSize; i++) {
        LiveActor* actor = table->mRequests[i];
        ActorExecuteInfo* info = actor->mActorExecuteInfo;
        for (s32 j = 0; j < info->mDrawerCount; j++) {
            info->mDrawers[j]->addModel(actor->mModelKeeper->mModelCafe);
        }
    }

    table->mSize = 0;
}

/**
 * Removes the models of all actors requested for draw removal from their drawers.
 */
void ExecuteRequestKeeper::executeRequestActorDrawAllOff() {
    ExecuteRequestTable* table = mRequestTables[Request_RemoveFromDraw];
    for (s32 i = 0; i < table->mSize; i++) {
        LiveActor* actor = table->mRequests[i];
        ActorExecuteInfo* info = actor->mActorExecuteInfo;
        for (s32 j = 0; j < info->mDrawerCount; j++) {
            info->mDrawers[j]->removeModel(actor->mModelKeeper->mModelCafe);
        }
    }

    table->mSize = 0;
}

/**
 * Adds the models of all actors requested for immediate drawing to their drawers.
 */
void ExecuteRequestKeeper::executeRequestActorDrawAllOnImmediate() {
    ExecuteRequestTable* table = mRequestTables[Request_DrawImmediate];
    for (s32 i = 0; i < table->mSize; i++) {
        LiveActor* actor = table->mRequests[i];
        ActorExecuteInfo* info = actor->mActorExecuteInfo;
        for (s32 j = 0; j < info->mDrawerCount; j++) {
            info->mDrawers[j]->addModel(actor->mModelKeeper->mModelCafe);
        }
    }

    table->mSize = 0;
}

/**
 * Queues an execute request for an actor, cancelling the opposite requests.
 * @param pActor The actor.
 * @param requestType The request type.
 */
void ExecuteRequestKeeper::request(LiveActor* pActor, s32 requestType) {
    ExecuteRequestTable* addTable = mRequestTables[requestType];
    ExecuteRequestTable* removeTable = nullptr;
    ExecuteRequestTable* removeTable2 = nullptr;

    switch (requestType) {
    case Request_Movement:
        removeTable = mRequestTables[Request_RemoveFromMovement];
        break;
    case Request_RemoveFromMovement:
        removeTable = mRequestTables[Request_Movement];
        break;
    case Request_Draw:
        removeTable = mRequestTables[Request_RemoveFromDraw];
        removeTable2 = mRequestTables[Request_DrawImmediate];
        break;
    case Request_RemoveFromDraw:
        removeTable = mRequestTables[Request_Draw];
        removeTable2 = mRequestTables[Request_DrawImmediate];
        break;
    case Request_DrawImmediate:
        removeTable = mRequestTables[Request_Draw];
        removeTable2 = mRequestTables[Request_RemoveFromDraw];
        break;
    }

    removeTable->removeRequest(pActor);
    if (removeTable2) {
        removeTable2->removeRequest(pActor);
    }

    addTable->addRequest(pActor);
}
}  // namespace al
