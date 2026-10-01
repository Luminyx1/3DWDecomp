#pragma once

#include <framework/nx/seadGameFrameworkNx.h>
#include <gfx/seadDrawContext.h>

namespace agl {
class DrawContext;
}

namespace sead {
class DrawContext;
}

namespace al {
class GameFrameworkNx : public sead::GameFrameworkNx {
public:
    static sead::DrawContext* getDrawContext() { return sInstance->mDrawContext; }

    static agl::DrawContext* getAglDrawContext() {
        return reinterpret_cast<agl::DrawContext*>(sInstance->mDrawContext);
    }

    static GameFrameworkNx* sInstance;

    sead::DrawContext* mDrawContext;
    u8 _220[0x27b - 0x220];
    bool _27b;
    bool _27c;
};
}  // namespace al
