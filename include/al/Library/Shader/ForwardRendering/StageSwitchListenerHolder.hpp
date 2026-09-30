#pragma once

#include <basis/seadTypes.h>

namespace al {
class StageSwitchAccesser;
class StageSwitchListener;
class StageSwitchListenerList;

class StageSwitchListenerHolder {
public:
    class RequestList {
    public:
        void init(s32 num);
        void addRequest(StageSwitchListenerList* pList);
        void removeRequest(StageSwitchListenerList* pList);
        void update();

        StageSwitchListenerList** mArray = nullptr;
        s32 mCount = 0;
    };

    StageSwitchListenerHolder(s32 switchNum);

    void add(StageSwitchListener* pListener, StageSwitchAccesser* pAccesser);
    void requestChange(s32 switchNo, bool isOn);
    void instantUpdate(s32 switchNo);
    void movement();

private:
    RequestList mRequestListA;
    RequestList mRequestListB;
    StageSwitchListenerList* mListenerLists = nullptr;
    RequestList* mCurrentRequestList = nullptr;
    RequestList* mNextRequestList = nullptr;
    s32 mSwitchNum;
};

static_assert(sizeof(StageSwitchListenerHolder) == 0x40);
}  // namespace al
