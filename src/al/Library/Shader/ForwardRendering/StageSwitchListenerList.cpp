#include "Library/Shader/ForwardRendering/StageSwitchListenerList.hpp"

#include "Library/StageSwitch/StageSwitchFunctorListener.hpp"

namespace al {
/**
 * Constructs an empty listener list without a pending request.
 */
StageSwitchListenerList::StageSwitchListenerList()
    : mHead(nullptr), mTail(nullptr), mRequestIndex(-1), mIsOn(false), mIsRequestOn(false) {}

/**
 * Appends a listener to the end of the list.
 * @param pListener Listener to add.
 */
void StageSwitchListenerList::addListener(StageSwitchListener* pListener) {
    Node* node = new Node{pListener, nullptr};

    if (!mHead) {
        mHead = node;
        mTail = node;
        return;
    }

    mTail->mNext = node;
    mTail = node;
}

/**
 * Notifies every listener when the requested switch state differs from the current one.
 */
void StageSwitchListenerList::update() {
    if (mIsOn != mIsRequestOn) {
        if (mIsRequestOn) {
            for (Node* node = mHead; node; node = node->mNext) {
                node->mListener->listenOn();
            }
        } else {
            for (Node* node = mHead; node; node = node->mNext) {
                node->mListener->listenOff();
            }
        }
    }

    mIsOn = mIsRequestOn;
}

/**
 * Sets the index of this list in the pending request list.
 * @param index Request index, or -1 when not requested.
 */
void StageSwitchListenerList::setRequestIndex(s32 index) {
    mRequestIndex = index;
}

/**
 * Requests a new switch state.
 * @param isOn Requested switch state.
 */
void StageSwitchListenerList::request(bool isOn) {
    mIsRequestOn = isOn;
}
}  // namespace al
