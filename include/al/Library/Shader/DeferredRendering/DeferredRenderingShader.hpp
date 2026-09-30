#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace nn::g3d {
class ResShadingModel;
class ResShaderProgram;
}  // namespace nn::g3d

namespace al {
class GpuMemAllocator;

/**
 * @brief Picks and sets up the deferred rendering shader variation from option values.
 * Partial layout: only what other units use so far.
 */
class DeferredRenderingShader {
public:
    DeferredRenderingShader(GpuMemAllocator* pAllocator, bool isUnused);

    void setup(bool isA, bool isB);
    void activate();

    const nn::g3d::ResShadingModel* getShadingModel() const { return mShadingModel; }
    const nn::g3d::ResShaderProgram* getShaderProgram() const { return mShaderProgram; }

private:
    char** mOptionValueBuffers;
    sead::BufferedSafeString** mOptionValues;
    void* _10;
    const nn::g3d::ResShadingModel* mShadingModel;
    const nn::g3d::ResShaderProgram* mShaderProgram;
    void* _28;
    void* _30;
    GpuMemAllocator* mAllocator;
    void* _40;
    s32 _48;
    void* _50;
};

static_assert(sizeof(DeferredRenderingShader) == 0x58);

}  // namespace al
