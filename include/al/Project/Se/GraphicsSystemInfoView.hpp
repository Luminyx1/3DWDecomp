#pragma once

#include <basis/seadTypes.h>

namespace al {
class SkyboxDirector;

/// Partial view of al::GraphicsSystemInfo exposing the members used by the Se folder.
struct GraphicsSystemInfoView {
    u8 _0[0x80];
    SkyboxDirector* mSkyboxDirector;  // _80
};
}  // namespace al
