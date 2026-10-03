#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <container/seadSafeArray.h>
#include <gfx/seadCamera.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>
#include <utility/aglParameter.h>
#include <utility/aglParameterObj.h>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglShaderEnum.h"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace agl {
class TextureData;
}  // namespace agl

namespace al {
class ActorInitInfo;
class GraphicsSystemInfo;
class LiveActor;
class LiveActorKit;
class MirrorActorBase;

/**
 * Mirror parameters of a graphics area.
 */
class MirrorParam {
public:
    void init();
    bool operator==(const MirrorParam& rOther) const;
    MirrorParam& operator=(const MirrorParam& rOther);
    void interp(const MirrorParam& rA, const MirrorParam& rB, f32 rate);

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

    s32 getMirrorTexWidthScale() const { return *mMirrorTexWidthScale; }

    s32 getMirrorTexHeightScale() const { return *mMirrorTexHeightScale; }

    const char* getMirror1ActorName() const { return mMirror1ActorName->cstr(); }

    const char* getMirror2ActorName() const { return mMirror2ActorName->cstr(); }

    f32 getMirror1ActorOffsetY() const { return *mMirror1ActorOffsetY; }

    f32 getMirror2ActorOffsetY() const { return *mMirror2ActorOffsetY; }

    s32 getBlurType() const { return *mBlurType; }

    s32 getBlurCount() const { return *mBlurCount; }

    f32 getMirror1NearOffset() const { return *mMirror1NearOffset; }

    f32 getMirror2NearOffset() const { return *mMirror2NearOffset; }

private:
    agl::utl::ParameterObj mParamObj;
    agl::utl::Parameter<s32> mMirrorTexWidthScale;
    agl::utl::Parameter<s32> mMirrorTexHeightScale;
    agl::utl::Parameter<sead::FixedSafeString<256>> mMirror1ActorName;
    agl::utl::Parameter<sead::FixedSafeString<256>> mMirror2ActorName;
    agl::utl::Parameter<f32> mMirror1ActorOffsetY;
    agl::utl::Parameter<f32> mMirror2ActorOffsetY;
    agl::utl::Parameter<s32> mBlurType;
    agl::utl::Parameter<s32> mBlurCount;
    agl::utl::Parameter<f32> mMirror1NearOffset;
    agl::utl::Parameter<f32> mMirror2NearOffset;
};

static_assert(sizeof(MirrorParam) == 0x390);

/**
 * Renders the mirror (planar reflection) views of up to two mirror actors chosen by the graphics
 * areas.
 */
class ShaderMirrorDirector : public GraphicsParamRequestInterpKeeper<MirrorParam> {
public:
    struct MirrorActorInfo {
        LiveActor* actor;
        sead::FixedSafeString<256> name;
    };

    static_assert(sizeof(MirrorActorInfo) == 0x120);

    ShaderMirrorDirector(GraphicsSystemInfo* pInfo, LiveActorKit* pKit);
    ~ShaderMirrorDirector();

    void pushBackMirrorActor(LiveActor* pActor, const ActorInitInfo& rInfo);
    bool isEnable() const;
    agl::TextureData* getMirrorTextureData(s32 textureId) const;
    void activateMirrorTexture(s32 textureId) const;
    s32 tryFindMirrorTextureId(const LiveActor* pActor) const;
    LiveActor* findMirrorActor(const char* pName) const;
    s32 findMirrorActorIndex(const char* pName) const;
    void calcMirrorRenderBufferSize(sead::Vector2i* pSize) const;
    void allocRenderBuffer();
    void freeRenderBuffer();
    f32 getRenderingCameraNearOffset() const;
    const sead::LookAtCamera* getRenderingCamera() const;
    void startRendering();
    agl::ShaderMode endRendering(agl::ShaderMode shaderMode);
    const sead::LookAtCamera* getMirrorCamera(const LiveActor* pActor) const;
    void movement();

    const agl::RenderBuffer& getRenderBuffer() const { return mRenderBuffer; }

    agl::TextureData* getDepthTexture() const { return mDepthTexture; }

    agl::TextureData* getColorTexture() const { return mColorTexture; }

private:
    MirrorActorBase* getRenderingActor() const {
        return mRenderingIndex == 0 ? mMirrorActors[0] : mMirrorActors[1];
    }

    sead::FixedObjArray<MirrorActorInfo, 64> mMirrorActorInfos;
    LiveActorKit* mLiveActorKit;
    sead::SafeArray<sead::LookAtCamera, 2> mCameras;
    sead::SafeArray<bool, 2> mIsHideModel;
    sead::SafeArray<MirrorActorBase*, 2> mMirrorActors = {};
    s32 mRenderingIndex = -1;
    agl::TextureData* mColorTexture = nullptr;
    agl::TextureData* mDepthTexture = nullptr;
    agl::RenderBuffer mRenderBuffer;
    agl::RenderTargetColor mColorTarget;
    agl::RenderTargetDepth mDepthTarget;
    agl::TextureData* mOtherTexture = nullptr;
    sead::Vector2i mRenderBufferSize;
};

static_assert(sizeof(ShaderMirrorDirector) == 0x62b0);

}  // namespace al

namespace MirrorFunction {
al::ShaderMirrorDirector* getShaderMirrorDirector(const al::LiveActor* pActor);
}  // namespace MirrorFunction
