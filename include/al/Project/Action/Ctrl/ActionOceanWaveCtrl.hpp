#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;

    /// Starts the ocean waves of the current action.
    class ActionOceanWaveCtrl {
    public:
        static ActionOceanWaveCtrl* tryCreate(LiveActor* pActor);
        void startAction(const char* pActionName);
        void update(f32 frame, f32 frameRate);
    };
};
