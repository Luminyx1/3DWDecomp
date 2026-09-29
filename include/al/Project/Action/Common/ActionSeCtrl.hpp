#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioKeeper;
class SeKeeper;
class SeUserInfo;

class ActionSeCtrl {
public:
    static ActionSeCtrl* tryCreate(AudioKeeper*);

    ActionSeCtrl(SeKeeper*, const SeUserInfo*);

    void init();
    void resetAction();
    void startAction(const char*);
    void stopAction();
    void resetState();
    void update(f32, f32);
};
}  // namespace al
