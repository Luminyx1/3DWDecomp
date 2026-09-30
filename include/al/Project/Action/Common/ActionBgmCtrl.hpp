#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioKeeper;

class ActionBgmCtrl {
public:
    static ActionBgmCtrl* tryCreate(AudioKeeper* pAudioKeeper);

    void startAction(const char* pActionName);
    void update(f32 frame, f32 frameRate);
    void tryPrepareActionFirstBgm(const char* pActionName, bool isForce, s32 unk);
};
}  // namespace al
