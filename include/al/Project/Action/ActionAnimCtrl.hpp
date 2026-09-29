#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;

    /// Plays the animations of an actor's actions.
    class ActionAnimCtrl {
    public:
        static ActionAnimCtrl* tryCreate(LiveActor* pActor, const char* pArchiveName, const char* pActionListName);

        bool start(const char* pActionName);
    };

    /// Per-action animation data read from the actor's action list.
    struct ActionAnimDataInfo {
        ActionAnimDataInfo();

        const char* mAnimName;      // _0
        f32 mRate;                  // _8
        bool _C;
        bool _D;
    };

    /// Animation info for one action; only the leading members are known so far.
    struct ActionAnimCtrlInfo {
        ActionAnimCtrlInfo(s32);

        const char* mActionName;    // _0
    };
};

namespace alActionFunction {
    const char* getAnimName(const al::ActionAnimCtrlInfo* pCtrlInfo, const al::ActionAnimDataInfo* pDataInfo);
};
