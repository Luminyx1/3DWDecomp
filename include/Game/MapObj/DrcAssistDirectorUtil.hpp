#pragma once

#include <math/seadVector.h>

namespace al {
    class HitSensor;
    class IUseSceneObjHolder;
    class LiveActor;
    class ScreenPointer;
};

namespace rc {
    al::LiveActor* findDrcAssistDirectorPlayer(const al::IUseSceneObjHolder*,
                                               const al::ScreenPointer*);
    al::LiveActor* findDrcAssistDirectorPlayer(const al::IUseSceneObjHolder*,
                                               const al::HitSensor*);
    al::LiveActor* findDrcAssistDirectorTouchPointer(const al::IUseSceneObjHolder*,
                                                     const al::ScreenPointer*);
    bool tryCalcTouchPointerSlideDirOnWorldByPointer(sead::Vector3f*, const al::LiveActor*);
    void releaseAllTouchPointerHoldItem(const al::LiveActor* pActor);
};