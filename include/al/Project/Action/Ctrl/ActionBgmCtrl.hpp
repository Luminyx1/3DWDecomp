#pragma once

#include <basis/seadTypes.h>

namespace al {
    class AudioKeeper;

    /// Plays the BGM of the current action.
    class ActionBgmCtrl {
    public:
        static ActionBgmCtrl* tryCreate(AudioKeeper* pKeeper);
        void startAction(const char* pActionName);
        void update(f32 frame, f32 frameRate);
    };
};
