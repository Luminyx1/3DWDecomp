#include "Library/Shader/ForwardRendering/StageSwitchListenerHolder.hpp"

#include "Library/Shader/ForwardRendering/StageSwitchListenerList.hpp"
#include "Library/StageSwitch/StageSwitchAccesser.hpp"

namespace al {
/**
 * Allocates the request array.
 * @param num Maximum number of requests.
 */
void StageSwitchListenerHolder::RequestList::init(s32 num) {
    mArray = new StageSwitchListenerList*[num];
    mCount = 0;
    for (s32 i = 0; i < num; i++) {
        mArray[i] = nullptr;
    }
}

/**
 * Adds a listener list to the front of the requests.
 * @param pList Listener list to add.
 */
void StageSwitchListenerHolder::RequestList::addRequest(StageSwitchListenerList* pList) {
    if (mCount != 0) {
        mArray[mCount] = mArray[0];
        mArray[mCount]->setRequestIndex(mCount);
        mArray[0] = pList;
        pList->setRequestIndex(0);
    } else {
        mArray[0] = pList;
        mArray[0]->setRequestIndex(0);
    }

    mCount++;
}

/**
 * Removes a listener list from the requests.
 * @param pList Listener list to remove.
 */
void StageSwitchListenerHolder::RequestList::removeRequest(StageSwitchListenerList* pList) {
    for (s32 i = 0; i < mCount; i++) {
        if (mArray[i] == pList) {
            if (i < mCount - 1) {
                mArray[i] = mArray[mCount - 1];
            }

            mCount--;
            return;
        }
    }
}

/**
 * Updates every requested listener list and clears the requests.
 */
void StageSwitchListenerHolder::RequestList::update() {
    for (s32 i = 0; i < mCount; i++) {
        mArray[i]->setRequestIndex(-1);
    }

    for (s32 i = 0; i < mCount; i++) {
        mArray[i]->update();
        mArray[i] = nullptr;
    }

    mCount = 0;
}

/**
 * Constructs the holder with one listener list per switch.
 * @param switchNum Number of switches.
 */
StageSwitchListenerHolder::StageSwitchListenerHolder(s32 switchNum) : mSwitchNum(switchNum) {
    mListenerLists = new StageSwitchListenerList[switchNum];
    mRequestListA.init(mSwitchNum);
    mRequestListB.init(mSwitchNum);
    mCurrentRequestList = &mRequestListA;
    mNextRequestList = &mRequestListB;
}

/**
 * Registers a listener for the switch of an accesser.
 * @param pListener Listener to add.
 * @param pAccesser Accesser of the switch.
 */
void StageSwitchListenerHolder::add(StageSwitchListener* pListener, StageSwitchAccesser* pAccesser) {
    s32 switchNo = pAccesser->getSwitchNo();
    mListenerLists[switchNo].addListener(pListener);
}

/**
 * Requests a switch state change, notified on the next movement.
 * @param switchNo Switch number.
 * @param isOn New switch state.
 */
void StageSwitchListenerHolder::requestChange(s32 switchNo, bool isOn) {
    StageSwitchListenerList* list = &mListenerLists[switchNo];
    if (list->isEmpty()) {
        return;
    }

    list->request(isOn);
    if (list->getRequestIndex() < 0) {
        mNextRequestList->addRequest(list);
    }
}

/**
 * Immediately notifies the listeners of a pending switch change.
 * @param switchNo Switch number.
 */
void StageSwitchListenerHolder::instantUpdate(s32 switchNo) {
    StageSwitchListenerList* list = &mListenerLists[switchNo];
    if (list->isEmpty() || list->getRequestIndex() < 0) {
        return;
    }

    list->update();
    mNextRequestList->removeRequest(list);
    list->setRequestIndex(-1);
}

/**
 * Swaps the request lists and notifies the listeners of the pending changes.
 */
void StageSwitchListenerHolder::movement() {
    RequestList* list = mNextRequestList;
    mNextRequestList = mCurrentRequestList;
    mCurrentRequestList = list;
    list->update();
}
}  // namespace al
