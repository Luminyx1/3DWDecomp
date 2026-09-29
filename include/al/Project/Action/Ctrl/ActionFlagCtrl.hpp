#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;

    /// Updates the actor flags of the current action.
    class ActionFlagCtrl {
    public:
        static ActionFlagCtrl* tryCreate(LiveActor* pActor, const char* pActionListName);
        void initPost();
        void start(const char* pActionName);
        void update(f32 frame, f32 frameRate);
    };
};
