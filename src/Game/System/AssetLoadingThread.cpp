#include "System/AssetLoadingThread.hpp"
#include "Library/System/GameSystemInfo.hpp"

/**
 * @brief Poll an asset-loading event without blocking.
 * @param type Load category index from 0 through 3.
 * @return True when the corresponding event is signaled.
 */
bool rc::AssetLoadingThread::isLoading(LOAD_TYPE type) {
    return mpLoadingEvents[type].wait(sead::TickSpan(0));
}

/**
 * @brief Poll an asset-loading event without blocking.
 * @param type Load category index from 0 through 3.
 * @return True when the corresponding event is signaled.
 */
bool rc::AssetLoadingThread::isLoadDone(LOAD_TYPE type) { return mpDoneEvents[type].wait(sead::TickSpan(0)); }

/**
 * @brief Check whether assets are loading or already loaded.
 * @param type Load category index from 0 through 3.
 * @return True when either loading or completion is signaled.
 */
bool rc::AssetLoadingThread::isLoadOrLoading(LOAD_TYPE type) { return isLoading(type) || isLoadDone(type); }

/**
 * @brief Toggle fast loading and release the CPU boost when disabled.
 * @param disable True to disable fast loading; false to enable it.
 */
void rc::AssetLoadingThread::disableFastLoad(bool disable) {
    if (mFastLoad == disable) {
        mFastLoad = !disable;
        if (!mFastLoad) {
            al::setCpuBoost(false, false);
        }
    }
}
