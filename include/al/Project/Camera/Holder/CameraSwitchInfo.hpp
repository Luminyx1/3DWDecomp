#pragma once

#include <basis/seadTypes.h>

namespace al {
/// Describes a pending camera switch request.
class CameraSwitchInfo {
public:
    CameraSwitchInfo();

    void reset();

    void* _0;   // _0
    s32 _8;     // _8
    bool _C;    // _C
    bool _D;    // _D
    void* _10;  // _10
    void* _18;  // _18
    void* _20;  // _20
    void* _28;  // _28
    s32 _30;    // _30
};
}  // namespace al
