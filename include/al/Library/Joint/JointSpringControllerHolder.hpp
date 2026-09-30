#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;
class JointSpringController;
class LiveActor;

class JointSpringControllerHolder {
public:
    struct Entry {
        JointSpringController* controller = nullptr;
        const char* jointName = nullptr;
    };

    JointSpringControllerHolder();

    void init(s32 maxControllers);
    void init(LiveActor* pActor, const char* pFileName);
    void init(LiveActor* pActor, const ByamlIter& rIter);
    void addController(JointSpringController* pController, const char* pJointName);
    void offControlAll();
    void setControlRateAll(f32 rate);
    void onControllAll();
    void resetControlAll();
    void addControlRateAll(f32 rate);
    void subControlRateAll(f32 rate);

private:
    Entry* mEntries = nullptr;
    s32 mNum = 0;
    s32 mMaxNum = 0;
};

static_assert(sizeof(JointSpringControllerHolder) == 0x10);

}  // namespace al
