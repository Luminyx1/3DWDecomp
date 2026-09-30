#include "Library/Light/SSIIKeeper.hpp"

#include <cull/aglViewFrustumCulling.h>
#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <lighting/aglSSII.h>

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * @brief Sets up the SSII parameters with their defaults.
 */
void SSIIParam::init() {
    mDiffuseIntensity.init(0.0f, "DiffuseIntensity", "DiffuseIntensity", "Min=0.0f,Max=10.0f",
                           &mParamObj);
    mReflectionIntensity.init(0.0f, "ReflectionIntensity", "ReflectionIntensity",
                              "Min=0.0f,Max=10.0f", &mParamObj);
    mRenderingQuality.init(1, "RenderingQuality", "RenderingQuality", "Min=0, Max=4", &mParamObj);
    mEdgeQuality.init(0, "EdgeQuality", "EdgeQuality", "Min=0, Max=4", &mParamObj);
    mDiffuseReduceTypeLight.init(0, "DiffuseReduceTypeLight", "DiffuseReduceTypeLight",
                                 "Min=0, Max=4", &mParamObj);
    mReflectionReduceTypeLight.init(0, "ReflectionReduceTypeLight", "ReflectionReduceTypeLight",
                                    "Min=0, Max=4", &mParamObj);
}

/**
 * @brief Compares all values with another parameter set.
 */
bool SSIIParam::operator==(const SSIIParam& rOther) const {
    return *mDiffuseIntensity == *rOther.mDiffuseIntensity &&
           *mReflectionIntensity == *rOther.mReflectionIntensity &&
           *mRenderingQuality == *rOther.mRenderingQuality &&
           *mEdgeQuality == *rOther.mEdgeQuality &&
           *mDiffuseReduceTypeLight == *rOther.mDiffuseReduceTypeLight &&
           *mReflectionReduceTypeLight == *rOther.mReflectionReduceTypeLight;
}

/**
 * @brief Copies all values of another parameter set.
 */
SSIIParam& SSIIParam::operator=(const SSIIParam& rOther) {
    *mDiffuseIntensity = *rOther.mDiffuseIntensity;
    *mReflectionIntensity = *rOther.mReflectionIntensity;
    *mRenderingQuality = *rOther.mRenderingQuality;
    *mEdgeQuality = *rOther.mEdgeQuality;
    *mDiffuseReduceTypeLight = *rOther.mDiffuseReduceTypeLight;
    *mReflectionReduceTypeLight = *rOther.mReflectionReduceTypeLight;
    return *this;
}

/**
 * @brief Interpolates between two parameter sets (quality settings switch halfway).
 */
void SSIIParam::interp(const SSIIParam& rA, const SSIIParam& rB, f32 rate) {
    mDiffuseIntensity.copyLerp(rA.mDiffuseIntensity, rB.mDiffuseIntensity, rate);
    mReflectionIntensity.copyLerp(rA.mReflectionIntensity, rB.mReflectionIntensity, rate);
    *mDiffuseReduceTypeLight =
        lerpValueNew(*rA.mDiffuseReduceTypeLight, *rB.mDiffuseReduceTypeLight, rate);
    *mReflectionReduceTypeLight =
        lerpValueNew(*rA.mReflectionReduceTypeLight, *rB.mReflectionReduceTypeLight, rate);
    mRenderingQuality.copy(rate < 0.5f ? rA.mRenderingQuality : rB.mRenderingQuality);
    mEdgeQuality.copy(rate < 0.5f ? rA.mEdgeQuality : rB.mEdgeQuality);
}

/**
 * @brief Checks whether indirect diffuse is on.
 */
bool SSIIParam::isEnableDiffuse() const {
    return *mDiffuseIntensity > 0.0f;
}

/**
 * @brief Checks whether indirect reflection is on.
 */
bool SSIIParam::isEnableReflection() const {
    return *mReflectionIntensity > 0.0f;
}

/**
 * @brief Creates the SSII renderer for the given number of views.
 */
SSIIKeeper::SSIIKeeper(s32 viewNum, GraphicsSystemInfo* pInfo)
    : GraphicsParamRequestInterpKeeper(pInfo, 0x10, "SSII", "aglssii", nullptr) {
    mSSII = new agl::lght::SSII();
    mSSII->initialize(viewNum, nullptr);
}

/**
 * @brief Destroys the SSII renderer.
 */
SSIIKeeper::~SSIIKeeper() {
    delete mSSII;
}

/**
 * @brief Applies the current SSII parameters to the renderer.
 */
void SSIIKeeper::movement() {
    updateRequest();
    const SSIIParam& param = getCurrentParam();
    mSSII->setEnable(param.isEnableDiffuse() || param.isEnableReflection());
    mSSII->setEnableDiffuse(getCurrentParam().isEnableDiffuse());
    mSSII->setEnableReflection(getCurrentParam().isEnableReflection());
    mSSII->setDifIntensity(getCurrentParam().getDiffuseIntensity());
    mSSII->setRefIntensity(getCurrentParam().getReflectionIntensity());
    mSSII->setRedBufQuality(getCurrentParam().getRenderingQuality());
    mSSII->setSampleQuality(getCurrentParam().getEdgeQuality());
    mSSII->setDifReduceLevel(getCurrentParam().getDiffuseReduceTypeLight());
    mSSII->setRefReduceLevel(getCurrentParam().getReflectionReduceTypeLight());
}

/**
 * @brief Culls the SSII lights of a view.
 */
void SSIIKeeper::updateViewGPU(s32 viewIndex, const sead::Camera* pCamera,
                               const sead::Projection* pProjection) {
    agl::cull::ViewFrustumCulling culling;
    culling.update(pCamera->getMatrix(), *pProjection);
    mSSII->calcView(viewIndex, culling);
}

}  // namespace al
