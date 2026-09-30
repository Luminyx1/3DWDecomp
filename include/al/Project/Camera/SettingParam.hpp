#pragma once

#include <basis/seadTypes.h>

namespace al {
class SettingParam {
public:
    SettingParam();

    f32 _0;
    f32 _4;
    f32 _8;
    f32 _C;
    s32 _10;
    s32 _14;
    f32 _18;
    f32 _1C;
    f32 _20;
    f32 _24;
    f32 _28;
    f32 _2C;
    f32 _30;
    bool _34;
    bool _35;
    bool _36;
};

static_assert(sizeof(SettingParam) == 0x38);
}  // namespace al
