#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class LiveActor;

    /// Plays the pad rumbles and camera shakes of the current action.
    class ActionPadAndCameraCtrl {
    public:
        static ActionPadAndCameraCtrl* tryCreate(const LiveActor* pActor, const sead::Vector3f* pPos, const char* pActionListName);
        void startAction(const char* pActionName);
        void update(f32 frame, f32 frameRate);
    };
};
