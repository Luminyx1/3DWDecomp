#pragma once

#include <math/seadVector.h>

namespace al {
/// Scene wide state used to control the cameras.
class SceneCameraControlInfo {
public:
    SceneCameraControlInfo();

    void* _0;                // _0
    void* _8;                // _8
    void* _10;               // _10
    sead::Vector3f _18;      // _18
    void* _28;               // _28
    bool _30;                // _30
    void* _38;               // _38
    void* _40;               // _40
    void* _48;               // _48
    void* _50;               // _50
    void* _58;               // _58
    void* _60;               // _60
};
}  // namespace al
