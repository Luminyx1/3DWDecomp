#pragma once

#include <basis/seadTypes.h>

namespace al {
    class IUseEffectKeeper;

    /// Plays the effects of the current action.
    class ActionEffectCtrl {
    public:
        static ActionEffectCtrl* tryCreate(IUseEffectKeeper* pKeeper);
        void startAction(const char* pActionName);
        void update(f32 frame, f32 frameRate);
    };
};
