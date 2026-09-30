#pragma once

#include <basis/seadTypes.h>
#include <utility/aglParameter.h>
#include <utility/aglParameterObj.h>

#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace agl::lght {
class SSII;
}  // namespace agl::lght

namespace sead {
class Camera;
class Projection;
}  // namespace sead

namespace al {
class GraphicsSystemInfo;

/**
 * @brief Screen space indirect illumination settings.
 */
class SSIIParam {
public:
    void init();
    bool operator==(const SSIIParam& rOther) const;
    SSIIParam& operator=(const SSIIParam& rOther);
    void interp(const SSIIParam& rA, const SSIIParam& rB, f32 rate);
    bool isEnableDiffuse() const;
    bool isEnableReflection() const;

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

    f32 getDiffuseIntensity() const { return *mDiffuseIntensity; }
    f32 getReflectionIntensity() const { return *mReflectionIntensity; }
    s32 getRenderingQuality() const { return *mRenderingQuality; }
    s32 getEdgeQuality() const { return *mEdgeQuality; }
    s32 getDiffuseReduceTypeLight() const { return *mDiffuseReduceTypeLight; }
    s32 getReflectionReduceTypeLight() const { return *mReflectionReduceTypeLight; }

private:
    agl::utl::ParameterObj mParamObj;
    agl::utl::Parameter<f32> mDiffuseIntensity;
    agl::utl::Parameter<f32> mReflectionIntensity;
    agl::utl::Parameter<s32> mRenderingQuality;
    agl::utl::Parameter<s32> mEdgeQuality;
    agl::utl::Parameter<s32> mDiffuseReduceTypeLight;
    agl::utl::Parameter<s32> mReflectionReduceTypeLight;
};

static_assert(sizeof(SSIIParam) == 0xf0);

/**
 * @brief Drives agl's screen space indirect illumination from the requested SSII parameters.
 */
class SSIIKeeper : public GraphicsParamRequestInterpKeeper<SSIIParam> {
public:
    SSIIKeeper(s32 viewNum, GraphicsSystemInfo* pInfo);
    ~SSIIKeeper();

    void movement();
    void updateViewGPU(s32 viewIndex, const sead::Camera* pCamera,
                       const sead::Projection* pProjection);

    agl::lght::SSII* getSSII() const { return mSSII; }

private:
    agl::lght::SSII* mSSII = nullptr;
};

static_assert(sizeof(SSIIKeeper) == 0x718);

}  // namespace al
