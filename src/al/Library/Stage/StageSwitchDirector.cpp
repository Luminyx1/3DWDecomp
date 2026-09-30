#include "Library/StageSwitch/Core/StageSwitchDirector.hpp"

#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Shader/ForwardRendering/StageSwitchListenerHolder.hpp"
#include "Library/StageSwitch/StageSwitchAccesser.hpp"
#include "Library/StageSwitch/StageSwitchWatcher.hpp"
#include "Library/StageSwitch/StageSwitchWatcherHolder.hpp"

namespace al {
/**
 * Constructs the stage switch director and registers it as an executor.
 * @param pExecuteDirector execute director
 * @param isUseListenerHolder true to notify listeners through a listener holder instead of watchers
 */
StageSwitchDirector::StageSwitchDirector(ExecuteDirector* pExecuteDirector,
                                         bool isUseListenerHolder)
    : mSwitchInfos(nullptr), mMaxSwitchNum(330), mSwitchNum(0), mWatcherHolder(nullptr),
      mListenerHolder(nullptr), mCameraDirector(nullptr) {
    mSwitchInfos = new StageSwitchInfo[mMaxSwitchNum];
    if (isUseListenerHolder) {
        mListenerHolder = new StageSwitchListenerHolder(mMaxSwitchNum);
    } else {
        mWatcherHolder = new StageSwitchWatcherHolder(0x800);
    }
    registerExecutorUser(this, pExecuteDirector, "ステージスイッチディレクター");
}

/**
 * Gets the switch used by an accesser, allocating a new one if needed.
 * @param pAccesser switch accesser
 * @return switch number, or -1 if no switch is left
 */
s32 StageSwitchDirector::useSwitch(const StageSwitchAccesser* pAccesser) {
    PlacementId* placementId = pAccesser->mPlacementId;
    s32 switchNo = findSwitchNoFromObjId(placementId);
    if (switchNo < 0) {
        if (mSwitchNum < mMaxSwitchNum) {
            mSwitchInfos[mSwitchNum].mPlacementId = placementId;
            switchNo = mSwitchNum;
            mSwitchNum++;
        } else {
            switchNo = -1;
        }
    }
    return switchNo;
}

/**
 * Finds the switch of a switch object.
 * @param pId placement id of the switch object
 * @return switch number, or -1 if not found
 */
s32 StageSwitchDirector::findSwitchNoFromObjId(const PlacementId* pId) {
    if (!pId) {
        return 0;
    }
    for (s32 i = 0; i < mSwitchNum; i++) {
        if (PlacementId::isEqual(*mSwitchInfos[i].mPlacementId, *pId)) {
            return i;
        }
    }
    return -1;
}

/**
 * Turns a switch on.
 * @param pAccesser switch accesser
 */
void StageSwitchDirector::onSwitch(const StageSwitchAccesser* pAccesser) {
    s32 switchNo = pAccesser->mSwitchNo;
    if (switchNo < 0 || mSwitchNum <= switchNo) {
        return;
    }
    mSwitchInfos[switchNo].mIsOn = true;
    if (mListenerHolder) {
        mListenerHolder->requestChange(switchNo, true);
    }
}

/**
 * Turns a switch off.
 * @param pAccesser switch accesser
 */
void StageSwitchDirector::offSwitch(const StageSwitchAccesser* pAccesser) {
    s32 switchNo = pAccesser->mSwitchNo;
    if (switchNo < 0 || mSwitchNum <= switchNo) {
        return;
    }
    if (mListenerHolder) {
        mListenerHolder->requestChange(switchNo, false);
    }
    mSwitchInfos[switchNo].mIsOn = false;
}

/**
 * Checks whether a switch is on.
 * @param pAccesser switch accesser
 * @return true if the switch is on
 */
bool StageSwitchDirector::isOnSwitch(const StageSwitchAccesser* pAccesser) {
    s32 switchNo = pAccesser->mSwitchNo;
    if (switchNo < 0 || mSwitchNum <= switchNo) {
        return false;
    }
    return mSwitchInfos[switchNo].mIsOn;
}

/**
 * Notifies the listeners of a switch immediately.
 * @param pAccesser switch accesser
 */
void StageSwitchDirector::instantUpdate(StageSwitchAccesser* pAccesser) {
    s32 switchNo = pAccesser->mSwitchNo;
    if (switchNo < 0 || mSwitchNum <= switchNo) {
        return;
    }
    if (mWatcherHolder) {
        return;
    }
    mListenerHolder->instantUpdate(switchNo);
}

/**
 * Adds a listener for a switch.
 * @param pListener listener to notify
 * @param pAccesser switch accesser
 */
void StageSwitchDirector::addListener(StageSwitchListener* pListener,
                                      StageSwitchAccesser* pAccesser) {
    StageSwitchWatcher* watcher = new StageSwitchWatcher(pListener, pAccesser);
    if (mWatcherHolder) {
        mWatcherHolder->add(watcher);
        return;
    }
    mListenerHolder->add(pListener, pAccesser);
}

/**
 * Updates the switch watchers or listeners.
 */
void StageSwitchDirector::execute() {
    if (mWatcherHolder) {
        mWatcherHolder->movement();
        return;
    }
    mListenerHolder->movement();
}
}  // namespace al
