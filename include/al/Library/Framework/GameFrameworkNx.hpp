#pragma once

#include <framework/nx/seadGameFrameworkNx.h>
#include <gfx/seadDrawContext.h>

namespace sead {
class DrawContext;
}

namespace al {
class GameFrameworkNx : public sead::GameFrameworkNx {
public:
    static GameFrameworkNx* sInstance;

    sead::DrawContext* mDrawContext;
    u8 _220[0x27b - 0x220];
    bool _27b;
    bool _27c;
};
}  // namespace al
