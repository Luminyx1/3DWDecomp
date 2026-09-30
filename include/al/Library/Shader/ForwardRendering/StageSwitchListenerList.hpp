#pragma once

#include <basis/seadTypes.h>

namespace al {
class StageSwitchListener;

class StageSwitchListenerList {
public:
    struct Node {
        StageSwitchListener* mListener;
        Node* mNext;
    };

    StageSwitchListenerList();

    void addListener(StageSwitchListener* pListener);
    void update();
    void setRequestIndex(s32 index);
    void request(bool isOn);

    bool isEmpty() const { return mHead == nullptr; }
    s32 getRequestIndex() const { return mRequestIndex; }

private:
    Node* mHead;
    Node* mTail;
    s32 mRequestIndex;
    bool mIsOn;
    bool mIsRequestOn;
};

static_assert(sizeof(StageSwitchListenerList) == 0x18);
}  // namespace al
