#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseName;
class PlacementId;
class StageSwitchDirector;
class StageSwitchListener;

class StageSwitchAccesser {
public:
    enum SwitchKind : s32 { None = 0, Read = 1, Write = 2 };

    StageSwitchAccesser();

    bool init(StageSwitchDirector* pDirector, const char* pLinkName, const PlacementId& rId,
              bool isDisasterMode);
    bool isValid() const;
    void onSwitch();
    void offSwitch();
    bool isOnSwitch() const;
    StageSwitchDirector* getStageSwitchDirector() const;
    bool isEnableRead() const;
    bool isEnableWrite() const;
    bool isEqualSwitch(const StageSwitchAccesser* pOther) const;
    void doInstantResponse();
    void addListener(StageSwitchListener* pListener);

    const char* getLinkName() const { return mName; }
    s32 getSwitchNo() const { return mSwitchNo; }
    bool isDisasterMode() const { return mIsDisasterMode; }
    void setUseName(IUseName* pUseName) { _8 = pUseName; }

    inline void setThing(void* ptr) { _8 = static_cast<IUseName*>(ptr); }

    StageSwitchDirector* mStageSwitchDirector = nullptr;
    IUseName* _8 = nullptr;
    const char* mName = "";
    PlacementId* mPlacementId = nullptr;
    s32 mSwitchNo = -1;
    SwitchKind mSwitchKind = None;
    bool mIsDisasterMode;
};

class StageSwitchAccesserList {
public:
    StageSwitchAccesserList();
    StageSwitchAccesserList(const StageSwitchAccesser* pAccesser);

    const StageSwitchAccesser* mAccesser = nullptr;
    u64 _8 = 0;
};
}  // namespace al
