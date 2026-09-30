#pragma once

#include <basis/seadTypes.h>

#include "Library/Execute/IUseExecutor.hpp"

namespace al {
class CameraDirector_RS;
class ExecuteDirector;
class PlacementId;
class StageSwitchAccesser;
class StageSwitchListener;
class StageSwitchListenerHolder;
class StageSwitchWatcherHolder;

class StageSwitchInfo {
public:
    StageSwitchInfo();

    PlacementId* mPlacementId;
    bool mIsOn;
};

class StageSwitchDirector : public IUseExecutor {
public:
    StageSwitchDirector(ExecuteDirector* pExecuteDirector, bool isUseListenerHolder);

    void execute() override;

    s32 useSwitch(const StageSwitchAccesser* pAccesser);
    s32 findSwitchNoFromObjId(const PlacementId* pId);
    void onSwitch(const StageSwitchAccesser* pAccesser);
    void offSwitch(const StageSwitchAccesser* pAccesser);
    bool isOnSwitch(const StageSwitchAccesser* pAccesser);
    void instantUpdate(StageSwitchAccesser* pAccesser);
    void addListener(StageSwitchListener* pListener, StageSwitchAccesser* pAccesser);

    StageSwitchInfo* mSwitchInfos;
    s32 mMaxSwitchNum;
    s32 mSwitchNum;
    StageSwitchWatcherHolder* mWatcherHolder;
    StageSwitchListenerHolder* mListenerHolder;
    CameraDirector_RS* mCameraDirector;
};
}  // namespace al
