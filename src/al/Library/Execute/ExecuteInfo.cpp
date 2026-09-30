#include "Library/Execute/ActorExecuteInfo.hpp"

#include "Library/Model/ModelDrawerBase.hpp"

namespace al {
/**
 * Constructs an empty actor execute info.
 * @param pKeeper The execute request keeper.
 */
ActorExecuteInfo::ActorExecuteInfo(ExecuteRequestKeeper* pKeeper) : mRequestKeeper(pKeeper) {}

/**
 * Adds an updater.
 * @param pUpdater The updater.
 */
void ActorExecuteInfo::addUpdater(ExecutorActorExecuteBase* pUpdater) {
    mUpdaters[mUpdaterCount] = pUpdater;
    mUpdaterCount++;
}

/**
 * Adds a model drawer.
 * @param pDrawer The drawer.
 */
void ActorExecuteInfo::addDrawer(ModelDrawerBase* pDrawer) {
    mDrawers[mDrawerCount] = pDrawer;
    mDrawerCount++;
}

/**
 * Removes a model drawer.
 * @param pDrawer The drawer.
 */
void ActorExecuteInfo::removeDrawer(ModelDrawerBase* pDrawer) {
    for (s32 i = 0; i < mDrawerCount; i++) {
        if (mDrawers[i] == pDrawer) {
            mDrawerCount--;

            for (s32 j = i; j < mDrawerCount; j++) {
                mDrawers[j] = mDrawers[j + 1];
            }

            mDrawers[mDrawerCount] = nullptr;
            return;
        }
    }
}

/**
 * Removes the first removeable model drawer.
 * @return The removed drawer or nullptr.
 */
ModelDrawerBase* ActorExecuteInfo::removeOptDrawer() {
    for (s32 i = 0; i < mDrawerCount; i++) {
        if (mDrawers[i]->isRemoveable()) {
            ModelDrawerBase* drawer = mDrawers[i];
            mDrawerCount--;

            for (s32 j = i; j < mDrawerCount; j++) {
                mDrawers[j] = mDrawers[j + 1];
            }

            mDrawers[mDrawerCount] = nullptr;
            return drawer;
        }
    }

    return nullptr;
}

/**
 * Constructs an empty execute request info.
 */
ExecuteRequestInfo::ExecuteRequestInfo() : _0(nullptr) {}
}  // namespace al
