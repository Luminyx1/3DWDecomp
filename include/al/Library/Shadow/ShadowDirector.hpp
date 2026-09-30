#pragma once

#include <basis/seadTypes.h>

namespace al {
class ShadowMaskKeeper;

class DepthShadowDrawer;

class ShadowDirector {
public:
    void endInit();

    ShadowMaskKeeper* getShadowMaskKeeper() const { return mShadowMaskKeeper; }

    void* _0;
    ShadowMaskKeeper* mShadowMaskKeeper;
    DepthShadowDrawer* mDepthShadowDrawer;
};

}  // namespace al
