#pragma once

#include <container/seadRingBuffer.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
class FootPrint;
class FootPrintServer;
class LiveActor;

class FootPrintHolder {
public:
    struct Timing {
        s32 frame;
        sead::Vector3f offset;
    };

    struct Action {
        const char* name;
        u32 timingMax;
        u32 timingNum;
        Timing* timings;
    };

    FootPrintHolder(LiveActor* pActor, const char* pArchiveName, FootPrintServer* pServer);

    void createActionList();
    void update();
    void appearFootPrint(const sead::Vector3f& rOffset);
    FootPrint* findDeadFootPrint();
    FootPrint* findDeadFootPrintByForce();
    const char* getCharacterName() const;
    const char* getMetamorphosisName() const;
    void createTimingList(Action* pAction, ByamlIter* pIter);
    s32 calcMaxAppearNum() const;

    FootPrintServer* mServer;
    sead::RingBuffer<FootPrint*>* mFootPrints = nullptr;
    LiveActor* mActor;
    ByamlIter* mInfoIter = nullptr;
    s32 mActionMax = 0;
    u32 mActionNum = 0;
    Action** mActions = nullptr;
    const char* mPrevActionName = nullptr;
    f32 mPrevActionFrame = 0.0f;
    const char* mCharacterName = nullptr;
    const char* mMetamorphosisName = nullptr;
};

static_assert(sizeof(FootPrintHolder) == 0x50);
}  // namespace al
