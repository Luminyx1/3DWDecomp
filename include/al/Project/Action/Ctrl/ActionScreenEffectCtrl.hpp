#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;

    /// Plays the screen effects of the current action.
    class ActionScreenEffectCtrl {
    public:
        static ActionScreenEffectCtrl* tryCreate(const LiveActor* pActor);
        void startAction(const char* pActionName);
        void update(f32 frame, f32 frameRate);
    };
};
