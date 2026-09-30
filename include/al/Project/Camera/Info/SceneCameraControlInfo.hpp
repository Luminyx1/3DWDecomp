#pragma once

#include <math/seadVector.h>

namespace al {

struct SceneCameraControlInfo {
    SceneCameraControlInfo();

    void* _0 = nullptr;
    void* _8 = nullptr;
    void* _10 = nullptr;
    sead::Vector3f _18 = sead::Vector3f::zero;
    void* _28 = nullptr;
    bool _30 = true;
    void* _38 = nullptr;
    void* _40 = nullptr;
    void* _48 = nullptr;
    void* _50 = nullptr;
    void* _58 = nullptr;
    void* _60 = nullptr;
};

}  // namespace al
