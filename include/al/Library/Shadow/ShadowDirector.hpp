#pragma once

#include <basis/seadTypes.h>

namespace agl::sdw {
class DepthShadow;
}  // namespace agl::sdw

namespace al {
class ShadowMaskKeeper;

class DepthShadowDrawer;

class ShadowDirector {
public:
    void endInit();
    agl::sdw::DepthShadow* getDepthShadow();

    ShadowMaskKeeper* getShadowMaskKeeper() const { return mShadowMaskKeeper; }

    void* _0;
    ShadowMaskKeeper* mShadowMaskKeeper;
    DepthShadowDrawer* mDepthShadowDrawer;
};

}  // namespace al
