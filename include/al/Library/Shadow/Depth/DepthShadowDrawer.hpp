#pragma once

#include <basis/seadTypes.h>

namespace agl::sdw {
class DepthShadow;
}

namespace al {

class DepthShadowDrawer {
public:
    agl::sdw::DepthShadow* getDepthShadow() const { return mDepthShadow; }

    bool isPreDraw() const { return mIsPreDraw; }

    s32 getDrawShadowIndex() const { return mDrawShadowIndex; }

private:
    u8 _0[0x18];
    agl::sdw::DepthShadow* mDepthShadow;
    u8 _20[0xac - 0x20];
    bool mIsPreDraw;
    s32 mDrawShadowIndex;
};

}  // namespace al
