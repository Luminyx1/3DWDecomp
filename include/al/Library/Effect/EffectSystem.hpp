#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Effect/EffectSystemInfo.hpp"

namespace agl {
class DrawContext;
class TextureData;
}  // namespace agl

namespace agl::sdw {
class DepthShadow;
}  // namespace agl::sdw

namespace sead {
class Heap;
class Viewport;
}  // namespace sead

namespace al {
class CameraDirector;
class EffectShaderHolder;
class ExecuteDirector;
class GraphicsSystemInfo;

class EffectSystem {
public:
    static EffectSystem* initializeSystem(agl::DrawContext* pDrawContext, sead::Heap* pHeap,
                                          bool isUsePatchResource);

    void addCalcEffect(u64 groupId);
    void initScene();
    void endInit();
    void startScene(ExecuteDirector* pExecuteDirector);
    void preprocess();
    void postprocess();
    void endScene();
    void setCameraDirector(CameraDirector* pCameraDirector);
    void setGraphicsSystemInfo(const GraphicsSystemInfo* pGraphicsSystemInfo);
    void updateEffect(const char* pGroupName) const;
    void calcEffectCompute() const;
    void drawEffectWithRenderPathAndCamPos(const sead::Matrix44f& rProjMtx,
                                           const sead::Matrix34f& rViewMtx,
                                           const sead::Vector3f& rCamPos, f32 near, f32 far,
                                           f32 fovy, const char* pGroupName, u32 renderPath,
                                           bool isCalcCompute) const;
    void drawEffectWithRenderPath(const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                                  f32 near, f32 far, f32 fovy, const char* pGroupName,
                                  u32 renderPath, bool isCalcCompute) const;
    void calcShadowClipVolume(agl::sdw::DepthShadow* pDepthShadow, const char* pGroupName,
                              u32 renderPath) const;
    bool isHasRenderingEmitter(u32 flag) const;

    EffectSystemInfo* getEffectSystemInfo() { return &mEffectSystemInfo; }
    EffectShaderHolder* getShaderHolder() const { return mShaderHolder; }
    agl::DrawContext* getDrawContext() const { return mDrawContext; }

    void* _0;
    EffectSystemInfo mEffectSystemInfo;
    u8 _28[0x390 - 0x28];
    EffectShaderHolder* mShaderHolder;
    u8 _398[0x3b8 - 0x398];
    agl::DrawContext* mDrawContext;
};
}  // namespace al

namespace alEffectSystemFunction {
void setDrawPathRenderStateSetCallbackSRT(const al::EffectSystem* pEffectSystem, bool isEnable);
void calcEffectCompute(const al::EffectSystem* pEffectSystem);
void setDrawPathRenderStateSetCallbackMRT(const al::EffectSystem* pEffectSystem);
void drawEffectDeferredShadowMask(const al::EffectSystem* pEffectSystem,
                                  const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                                  f32 near, f32 far, f32 fovy);
void drawEffectDeferredShadowMaskWithPos(const al::EffectSystem* pEffectSystem,
                                         const sead::Matrix44f& rProjMtx,
                                         const sead::Matrix34f& rViewMtx,
                                         const sead::Vector3f& rCamPos, f32 near, f32 far,
                                         f32 fovy);
void drawEffectDeferredShadowMaskPlayer(const al::EffectSystem* pEffectSystem,
                                        const sead::Matrix44f& rProjMtx,
                                        const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                                        f32 fovy);
void drawEffectDeferred(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy);
void drawEffectDeferredWithPos(const al::EffectSystem* pEffectSystem,
                               const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                               const sead::Vector3f& rCamPos, f32 near, f32 far, f32 fovy);
void drawEffectDeferredPlayer(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy);
void drawEffectForward(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                       const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy);
void drawEffectForwardPlayer(const al::EffectSystem* pEffectSystem,
                             const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                             f32 near, f32 far, f32 fovy);
void drawEffectForwardAfterFog(const al::EffectSystem* pEffectSystem,
                               const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                               f32 near, f32 far, f32 fovy);
void drawEffectForwardReduced(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy);
void drawEffectForwardReducedHDR(const al::EffectSystem* pEffectSystem,
                                 const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                                 f32 near, f32 far, f32 fovy);
void drawEffectIndirect(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy,
                        const sead::Viewport* pViewport);
void drawEffectIndirectPlayer(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy, const sead::Viewport* pViewport);
void drawEffectPostEffectBackground(const al::EffectSystem* pEffectSystem,
                                    const sead::Matrix44f& rProjMtx,
                                    const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy);
void drawEffectPostEffect(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                          const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy);
void drawEffectAfterFog(const al::EffectSystem* pEffectSystem, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy);
void drawEffectShadowCaster(const al::EffectSystem* pEffectSystem,
                            const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                            f32 near, f32 far, f32 fovy);
void drawEffectShadowReceiver(const al::EffectSystem* pEffectSystem,
                              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                              f32 near, f32 far, f32 fovy);
void drawEffect2D(const al::EffectSystem* pEffectSystem, const sead::Viewport* pViewport);
void updateEffect2D(al::EffectSystem* pEffectSystem);
void calcShadowClipVolume(const al::EffectSystem* pEffectSystem,
                          agl::sdw::DepthShadow* pDepthShadow);
void tryDeleteEmitterAndParticleOneTime(const al::EffectSystemInfo* pSystemInfo);
void setDepthTexture(const al::EffectSystem* pEffectSystem, const agl::TextureData* pTexture);
bool isHasRenderingEmitterInReduceBuffer(const al::EffectSystem* pEffectSystem);
bool isHasRenderingEmitterInReduceBufferHdr(const al::EffectSystem* pEffectSystem);
}  // namespace alEffectSystemFunction
