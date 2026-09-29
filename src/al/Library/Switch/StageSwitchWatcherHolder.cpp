#include "Library/StageSwitch/StageSwitchWatcherHolder.hpp"

#include "Library/StageSwitch/StageSwitchWatcher.hpp"

namespace al {
    /**
     * @brief Constructs a holder for a fixed number of watchers.
     * @param maxHolders The maximum number of watchers.
     */
    StageSwitchWatcherHolder::StageSwitchWatcherHolder(s32 maxHolders)
        : mNumHolders(0), mMaxHolders(maxHolders) {
        mWatchers = new StageSwitchWatcher*[maxHolders];
    }

    /**
     * @brief Adds a watcher to the holder.
     * @param pWatcher The watcher to add.
     */
    void StageSwitchWatcherHolder::add(StageSwitchWatcher* pWatcher) {
        mWatchers[mNumHolders] = pWatcher;
        mNumHolders++;
    }

    /**
     * @brief Updates all held watchers.
     */
    void StageSwitchWatcherHolder::movement() {
        for (s32 i = 0; i < mNumHolders; i++) {
            mWatchers[i]->update();
        }
    }
}  // namespace al
