#pragma once

#include <basis/seadTypes.h>

namespace al {
/// Scene wide camera settings shared by the camera posers.
class SettingParam {
public:
    SettingParam();

    f32 _0;    // _0
    f32 _4;    // _4
    f32 _8;    // _8
    f32 _C;    // _C
    s32 _10;   // _10
    s32 _14;   // _14
    f32 _18;   // _18
    f32 _1C;   // _1C
    f32 _20;   // _20
    f32 _24;   // _24
    f32 _28;   // _28
    f32 _2C;   // _2C
    f32 _30;   // _30
    bool _34;  // _34
    bool _35;  // _35
    bool _36;  // _36
};
}  // namespace al
