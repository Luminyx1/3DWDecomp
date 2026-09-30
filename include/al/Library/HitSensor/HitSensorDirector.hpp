#pragma once

#include <basis/seadTypes.h>

#include "Library/Execute/IUseExecutor.hpp"
#include "Project/Framework/MultiCoreQueueThread.hpp"

namespace al {
class ExecuteDirector;
class HitSensor;
class SensorHitGroup;

class HitSensorDirector : public IUseExecutor, public MultiCoreQueueExecutor {
public:
    class MultiThreadUpdate : public IUseExecutor {
    public:
        MultiThreadUpdate(ExecuteDirector* pExecuteDirector, HitSensorDirector* pDirector);
        virtual ~MultiThreadUpdate();

        void execute() override;
        void waitDone();

        HitSensorDirector* mDirector;
    };

    HitSensorDirector(ExecuteDirector* pExecuteDirector, s32 scale, MultiCoreQueueThread* pThread);

    void initGroup(HitSensor* pSensor);
    void trueExecute();
    void execute() override;
    const char* executorName() const override { return "HitSensorDirector"; }
    void executeOnThread() override;
    virtual ~HitSensorDirector();
    void executeHitCheckGroup(SensorHitGroup* pGroupA, SensorHitGroup* pGroupB) const;
    void executeHitCheck(HitSensor* pA, HitSensor* pB) const;
    void executeHitCheckInSameGroup(SensorHitGroup* pGroup) const;

    MultiThreadUpdate* mThreadUpdate = nullptr;
    MultiCoreQueueThread* mQueueThread;
    bool mIsExecuteDirect = true;
    SensorHitGroup* mPlayerGroup = nullptr;
    SensorHitGroup* mPlayerEyeGroup = nullptr;
    SensorHitGroup* mRideGroup = nullptr;
    SensorHitGroup* mEyeGroup = nullptr;
    SensorHitGroup* mSimpleGroup = nullptr;
    SensorHitGroup* mMapObjGroup = nullptr;
    SensorHitGroup* mCharacterGroup = nullptr;
};
}  // namespace al
