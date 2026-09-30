#include "Library/StageSwitch/StageSwitchWatcherHolder.hpp"

#include "Library/StageSwitch/StageSwitchWatcher.hpp"

namespace al {
/**
 * Constructs a holder for switch watchers.
 * @param maxNum maximum number of watchers
 */
StageSwitchWatcherHolder::StageSwitchWatcherHolder(s32 maxNum)
    : mNumHolders(0), mMaxHolders(maxNum) {
    mWatchers = new StageSwitchWatcher*[maxNum];
}

/**
 * Adds a watcher.
 * @param pWatcher watcher to add
 */
void StageSwitchWatcherHolder::add(StageSwitchWatcher* pWatcher) {
    mWatchers[mNumHolders] = pWatcher;
    mNumHolders++;
}

/**
 * Updates all watchers.
 */
void StageSwitchWatcherHolder::movement() {
    for (s32 i = 0; i < mNumHolders; i++) {
        mWatchers[i]->update();
    }
}
}  // namespace al
