#pragma once

#include "Library/Obj/Sky.hpp"

namespace sead {
class LookAtCamera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class SkyProjection : public Sky {
public:
    SkyProjection(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void appear() override;
    void kill() override;
    void control() override;

    void updateUniform();

    sead::LookAtCamera* mCamera0 = nullptr;
    sead::LookAtCamera* mCamera1 = nullptr;
    sead::PerspectiveProjection* mProjection0 = nullptr;
    sead::PerspectiveProjection* mProjection1 = nullptr;
    bool mIsEnableTexMtxSet = false;
    bool mIsEnableTexMtxSetUnder = false;
    f32 mSkyTexMtxV = 0.0f;
    f32 mSkyTexMtxVUnder = 0.0f;
    f32 mCloudTexMtxV = 0.0f;
    f32 mCloudTexMtxVUnder = 0.0f;
};
}  // namespace al
