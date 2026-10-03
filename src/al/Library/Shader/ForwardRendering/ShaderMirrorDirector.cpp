#include "Library/Shader/ForwardRendering/ShaderMirrorDirector.hpp"

#include <common/aglGPUMemAddr.h>
#include <common/aglShaderLocation.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglImageFilter2D.h>
#include <utility/aglPrimitiveTexture.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Execute/ExecuteDirector.hpp"
#include "Library/Execute/ExecuteRequestKeeper.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/MirrorActorBase.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsStressDirector.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace {
using namespace al;

/**
 * Places a mirror camera at the main camera reflected on the mirror plane of an actor.
 * @param pCamera Mirror camera to update.
 * @param pActor Mirror actor, or nullptr to leave the camera untouched.
 * @param offsetY Offset of the mirror plane along the actor's up direction.
 */
NOINLINE void updateMirrorCamera(sead::LookAtCamera* pCamera, LiveActor* pActor, f32 offsetY) {
    if (pActor == nullptr) {
        return;
    }

    sead::Vector3f up = sead::Vector3f::ez;
    calcUpDir(&up, pActor);
    normalizeOrZero(&up);
    sead::Vector3f planePos;
    planePos.setScaleAdd(offsetY, up, getTrans(pActor));
    const sead::Vector3f& cameraPos = getCameraPos(pActor);
    const sead::Vector3f& cameraLookAt = getCameraLookAt(pActor);

    if (up.dot(cameraLookAt - cameraPos) > 0.0f) {
        pCamera->setPos(cameraPos);
        pCamera->setAt(cameraLookAt);
    } else {
        sead::Vector3f at = cameraLookAt - planePos;
        sead::Vector3f pos = cameraPos - planePos;
        at -= up * (at.dot(up) * 2.0f);
        pos -= up * (pos.dot(up) * 2.0f);
        pCamera->setPos(planePos + pos);
        pCamera->setAt(planePos + at);
    }

    pCamera->updateViewMatrix();
}

/**
 * Copies a matrix element by element.
 * @param pDst Destination matrix.
 * @param rSrc Source matrix.
 */
inline void copyMatrix(sead::Matrix44f* pDst, const sead::Matrix44f& rSrc) {
    for (s32 j = 0; j < 4; j++) {
        for (s32 i = 0; i < 4; i++) {
            pDst->m[i][j] = rSrc.m[i][j];
        }
    }
}

/**
 * Blurs a texture with a separable gaussian filter, using a work texture of the same size.
 * @param pTexture Texture to blur.
 * @param pWorkTexture Work texture receiving the horizontal pass.
 * @param width Texture width.
 * @param height Texture height.
 * @param kernel Gaussian kernel.
 */
void blurTexture(agl::TextureData* pTexture, agl::TextureData* pWorkTexture, s32 width,
                 s32 height, agl::utl::ImageFilter2D::GaussianKernel kernel) {
    agl::TextureSampler colorSampler;
    colorSampler.applyTextureData(*pTexture);
    agl::RenderTargetColor renderTarget;
    renderTarget.applyTextureData(*pWorkTexture);
    agl::RenderBuffer renderBuffer;
    renderBuffer.setVirtualSize(sead::Vector2f(width, height));
    renderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    renderBuffer.setRenderTargetColorNullAll();
    renderBuffer.setRenderTargetColor(&renderTarget);
    renderBuffer.bind(GameFrameworkNx::getAglDrawContext());
    sead::Viewport viewport(renderBuffer);
    viewport.apply(GameFrameworkNx::getAglDrawContext(), renderBuffer);
    agl::utl::ImageFilter2D::drawGaussian(GameFrameworkNx::getAglDrawContext(), colorSampler,
                                          viewport, kernel, true, true, sead::Vector2f::zero);
    renderTarget.invalidateGPUCache(GameFrameworkNx::getAglDrawContext());

    agl::TextureSampler workSampler;
    workSampler.applyTextureData(*pWorkTexture);
    renderTarget.applyTextureData(*pTexture);
    renderBuffer.bind(GameFrameworkNx::getAglDrawContext());
    viewport.apply(GameFrameworkNx::getAglDrawContext(), renderBuffer);
    agl::utl::ImageFilter2D::drawGaussian(GameFrameworkNx::getAglDrawContext(), workSampler,
                                          viewport, kernel, false, true, sead::Vector2f::zero);
    renderTarget.invalidateGPUCache(GameFrameworkNx::getAglDrawContext());
}

}  // namespace

namespace al {

/**
 * Initializes all mirror parameters with their default values.
 */
void MirrorParam::init() {
    mMirrorTexWidthScale.init(106, "MirrorTexWidthScale", "幅", "Min=1,Max=160", &mParamObj);
    mMirrorTexHeightScale.init(60, "MirrorTexHeightScale", "高さ", "Min=1,Max=90", &mParamObj);
    mMirror1ActorName.init(sead::FixedSafeString<64>("No Name"), "Mirror1ActorName",
                           "鏡描画１のアクター名", &mParamObj);
    mMirror2ActorName.init(sead::FixedSafeString<64>("No Name"), "Mirror2ActorName",
                           "鏡描画２のアクター名", &mParamObj);
    mMirror1ActorOffsetY.init(0.0f, "Mirror1ActorOffsetY", "アクター１の鏡Ｙオフセット",
                              "Min=-1000.0f, Max=1000.0f", &mParamObj);
    mMirror2ActorOffsetY.init(0.0f, "Mirror2ActorOffsetY", "アクター２の鏡Ｙオフセット",
                              "Min=-1000.0f, Max=1000.0f", &mParamObj);
    mBlurType.init(-1, "BlurType", "ブラータイプ", "Min=0, Max=10", &mParamObj);
    mBlurCount.init(1, "BlurCount", "ブラー回数", "Min=1, Max=5", &mParamObj);
    mMirror1NearOffset.init(0.0f, "Mirror1NearOffset", "カメラ１ニアオフセット", &mParamObj);
    mMirror2NearOffset.init(0.0f, "Mirror2NearOffset", "カメラ２ニアオフセット", &mParamObj);
}

/**
 * Compares all parameter values with another mirror parameter.
 * @param rOther Parameter to compare with.
 * @return Whether all values are equal.
 */
bool MirrorParam::operator==(const MirrorParam& rOther) const {
    return *mMirrorTexWidthScale == *rOther.mMirrorTexWidthScale &&
           *mMirrorTexHeightScale == *rOther.mMirrorTexHeightScale &&
           isEqualString(mMirror1ActorName->cstr(), rOther.mMirror1ActorName->cstr()) &&
           isEqualString(mMirror2ActorName->cstr(), rOther.mMirror2ActorName->cstr()) &&
           *mMirror1ActorOffsetY == *rOther.mMirror1ActorOffsetY &&
           *mMirror2ActorOffsetY == *rOther.mMirror2ActorOffsetY &&
           *mBlurType == *rOther.mBlurType && *mBlurCount == *rOther.mBlurCount &&
           *mMirror1NearOffset == *rOther.mMirror1NearOffset &&
           *mMirror2NearOffset == *rOther.mMirror2NearOffset;
}

/**
 * Copies all parameters from another mirror parameter.
 * @param rOther Parameter to copy from.
 * @return This parameter.
 */
MirrorParam& MirrorParam::operator=(const MirrorParam& rOther) {
    *mMirrorTexWidthScale = *rOther.mMirrorTexWidthScale;
    *mMirrorTexHeightScale = *rOther.mMirrorTexHeightScale;
    mMirror1ActorName = rOther.mMirror1ActorName;
    mMirror2ActorName = rOther.mMirror2ActorName;
    mMirror1ActorOffsetY = rOther.mMirror1ActorOffsetY;
    mMirror2ActorOffsetY = rOther.mMirror2ActorOffsetY;
    mBlurType = rOther.mBlurType;
    mBlurCount = rOther.mBlurCount;
    *mMirror1NearOffset = *rOther.mMirror1NearOffset;
    *mMirror2NearOffset = *rOther.mMirror2NearOffset;
    return *this;
}

/**
 * Interpolates between two mirror parameters. Every parameter switches at the halfway point.
 * @param rA Parameter at rate 0.
 * @param rB Parameter at rate 1.
 * @param rate Interpolation rate.
 */
void MirrorParam::interp(const MirrorParam& rA, const MirrorParam& rB, f32 rate) {
    mMirrorTexWidthScale = rate < 0.5f ? rA.mMirrorTexWidthScale : rB.mMirrorTexWidthScale;
    mMirrorTexHeightScale = rate < 0.5f ? rA.mMirrorTexHeightScale : rB.mMirrorTexHeightScale;
    mMirror1ActorName = rate < 0.5f ? rA.mMirror1ActorName : rB.mMirror1ActorName;
    mMirror2ActorName = rate < 0.5f ? rA.mMirror2ActorName : rB.mMirror2ActorName;
    mMirror1ActorOffsetY = rate < 0.5f ? rA.mMirror1ActorOffsetY : rB.mMirror1ActorOffsetY;
    mMirror2ActorOffsetY = rate < 0.5f ? rA.mMirror2ActorOffsetY : rB.mMirror2ActorOffsetY;
    mBlurType = rate < 0.5f ? rA.mBlurType : rB.mBlurType;
    mBlurCount = rate < 0.5f ? rA.mBlurCount : rB.mBlurCount;
    mMirror1NearOffset.copy(rate < 0.5f ? rA.mMirror1NearOffset : rB.mMirror1NearOffset);
    mMirror2NearOffset.copy(rate < 0.5f ? rA.mMirror2NearOffset : rB.mMirror2NearOffset);
}

/**
 * Creates the mirror director.
 * @param pInfo Graphics system info.
 * @param pKit Actor kit of the scene.
 */
ShaderMirrorDirector::ShaderMirrorDirector(GraphicsSystemInfo* pInfo, LiveActorKit* pKit)
    : GraphicsParamRequestInterpKeeper<MirrorParam>(pInfo, 7, "MirrorRendering", "aglmirror",
                                                    "MirrorParam"),
      mLiveActorKit(pKit) {
    mRenderBufferSize.set(getCurrentParam().getMirrorTexWidthScale() * 8,
                          getCurrentParam().getMirrorTexHeightScale() * 8);
}

/**
 * Frees the other mirror texture.
 */
ShaderMirrorDirector::~ShaderMirrorDirector() {
    if (mOtherTexture != nullptr) {
        agl::utl::DynamicTextureAllocator::instance()->free(mOtherTexture);
        mOtherTexture = nullptr;
    }
}

/**
 * Registers an actor that can be chosen as mirror by name.
 * @param pActor Mirror actor.
 * @param rInfo Init info of the actor, used to build its unique name.
 */
void ShaderMirrorDirector::pushBackMirrorActor(LiveActor* pActor, const ActorInitInfo& rInfo) {
    PlacementId placementId;
    tryGetPlacementID(&placementId, rInfo.getPlacementInfo());
    MirrorActorInfo* info = mMirrorActorInfos.emplaceBack();
    info->actor = pActor;

    if (placementId.mUnitConfigName != nullptr) {
        info->name.format("%s_%s_%s_%s", pActor->getName(), placementId.mUnitConfigName,
                          placementId.mZoneID, placementId.mPlacementID);
    } else {
        info->name.format("%s_%s", pActor->getName(), placementId.mPlacementID);
    }
}

/**
 * Checks whether the first mirror actor is alive and visible.
 * @return Whether mirror rendering is enabled.
 */
bool ShaderMirrorDirector::isEnable() const {
    if (mMirrorActors[0] == nullptr) {
        return false;
    }

    if (!isAlive(mMirrorActors[0])) {
        return false;
    }

    return !isHideModel(mMirrorActors[0]);
}

/**
 * Gets the texture holding the reflection of a mirror.
 * @param textureId Mirror texture ID (0 or 1, 99 for none).
 * @return Mirror texture, or nullptr for no mirror.
 */
agl::TextureData* ShaderMirrorDirector::getMirrorTextureData(s32 textureId) const {
    switch (textureId) {
    case 99:
        return nullptr;
    case 0:
        if (mRenderingIndex == 0) {
            return mColorTexture;
        }

        break;
    case 1:
        if (mRenderingIndex == 1) {
            return mColorTexture;
        }

        break;
    }

    return mOtherTexture;
}

/**
 * Binds the mirror texture to the mirror sampler location.
 * @param textureId Mirror texture ID (0 or 1, 99 for none, negative for gray).
 */
void ShaderMirrorDirector::activateMirrorTexture(s32 textureId) const {
    if (textureId < 0) {
        agl::utl::PrimitiveTexture::instance()
            ->getTextureSampler(agl::utl::PrimitiveTexture::cType_Gray2D)
            ->activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationMirrorTex(), -1,
                       false);
        return;
    }

    agl::TextureData* texture = getMirrorTextureData(textureId);

    if (texture != nullptr && isEnable()) {
        agl::TextureSampler sampler(*texture);
        sampler.setMinLod(0.0f);
        sampler.setMaxLod(0.0f);
        sampler.setWrapX(1);
        sampler.setWrapY(1);
        sampler.activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationMirrorTex(), -1,
                         false);
        return;
    }

    agl::utl::PrimitiveTexture::instance()
        ->getTextureSampler(agl::utl::PrimitiveTexture::cType_Black2D)
        ->activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationAlphaProjMaskTex(), -1,
                   false);
    agl::utl::PrimitiveTexture::instance()
        ->getTextureSampler(agl::utl::PrimitiveTexture::cType_Black2D)
        ->activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationMirrorTex(), -1,
                   false);
}

/**
 * Gets the mirror texture ID of an actor.
 * @param pActor Actor to look up.
 * @return 0 or 1 for the first or second mirror, 99 if the actor is no current mirror.
 */
s32 ShaderMirrorDirector::tryFindMirrorTextureId(const LiveActor* pActor) const {
    if (mMirrorActors[0] == pActor) {
        return 0;
    }

    if (mMirrorActors[1] == pActor) {
        return 1;
    }

    return 99;
}

/**
 * Finds a registered mirror actor by its unique name.
 * @param pName Unique name of the actor.
 * @return Mirror actor, or nullptr if not found.
 */
LiveActor* ShaderMirrorDirector::findMirrorActor(const char* pName) const {
    s32 index = findMirrorActorIndex(pName);

    if (index == -1) {
        return nullptr;
    }

    return mMirrorActorInfos(index)->actor;
}

/**
 * Finds the index of a registered mirror actor by its unique name.
 * @param pName Unique name of the actor.
 * @return Index of the actor, or -1 if not found.
 */
s32 ShaderMirrorDirector::findMirrorActorIndex(const char* pName) const {
    if (pName == nullptr) {
        return -1;
    }

    s32 size = mMirrorActorInfos.size();

    for (s32 i = 0; i < size; i++) {
        if (isEqualString(mMirrorActorInfos[i]->name.cstr(), pName)) {
            return i;
        }
    }

    return -1;
}

/**
 * Gets the size of the mirror render buffer.
 * @param pSize Output size.
 */
void ShaderMirrorDirector::calcMirrorRenderBufferSize(sead::Vector2i* pSize) const {
    pSize->x = mRenderBufferSize.x;
    pSize->y = mRenderBufferSize.y;
}

/**
 * Allocates the mirror depth and color textures and sets up the mirror render buffer.
 */
void ShaderMirrorDirector::allocRenderBuffer() {
    bool isUsing16BitDepth =
        mGraphicsSystemInfo->getGraphicsStressDirector()->getCurrentParam().isUsing16BitDepth();
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    s32 width = mRenderBufferSize.x;
    s32 height = mRenderBufferSize.y;
    agl::GPUMemVoidAddr zCullBuffer;

    mDepthTexture = allocator->alloc(
        GameFrameworkNx::getAglDrawContext(), "Mirror Depth Target",
        isUsing16BitDepth ? agl::TextureFormat::cTextureFormat_Depth_16 :
                            agl::TextureFormat::cTextureFormat_Depth_32,
        width, height, 1, &zCullBuffer, agl::utl::DynamicTextureAllocator::cAllocateType_0, true,
        false);
    mDepthTarget.applyTextureData(*mDepthTexture, 0, 0);
    mDepthTarget.setZCullBuffer(agl::GPUMemVoidAddr(zCullBuffer));

    mColorTexture = allocator->alloc(GameFrameworkNx::getAglDrawContext(),
                                     "Mirror Deferred Shading Target",
                                     agl::TextureFormat::cTextureFormat_R11_G11_B10_float, width,
                                     height, 1, nullptr,
                                     agl::utl::DynamicTextureAllocator::cAllocateType_0, true,
                                     false);
    mColorTarget.applyTextureData(*mColorTexture);

    mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    mRenderBuffer.setRenderTargetColorNullAll();
    mRenderBuffer.setRenderTargetColor(&mColorTarget);
    mRenderBuffer.setRenderTargetDepth(&mDepthTarget);
}

/**
 * Frees the mirror textures. When a second mirror is active, the rendered texture is kept in
 * the other mirror texture first.
 */
void ShaderMirrorDirector::freeRenderBuffer() {
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();

    if (mMirrorActors[1] != nullptr) {
        s32 width = mRenderBufferSize.x;
        s32 height = mRenderBufferSize.y;

        bool isNeedAlloc = mOtherTexture == nullptr;

        if (!isNeedAlloc && (mOtherTexture->getWidth(0) != width ||
                             mOtherTexture->getHeight(0) != height)) {
            allocator->free(mOtherTexture);
            isNeedAlloc = true;
        }

        if (isNeedAlloc) {
            mOtherTexture = allocator->alloc(
                GameFrameworkNx::getAglDrawContext(), "Mirror Texture Other",
                agl::TextureFormat::cTextureFormat_R11_G11_B10_float, width, height, 1, nullptr,
                agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);
        }

        mColorTexture->copyToAll(GameFrameworkNx::getAglDrawContext(), mOtherTexture);
    }

    allocator->free(mColorTexture);
    allocator->free(mDepthTexture);
    mDepthTexture = nullptr;
    mColorTexture = nullptr;
}

/**
 * Gets the near clip offset of the mirror camera being rendered.
 * @return Near clip offset.
 */
f32 ShaderMirrorDirector::getRenderingCameraNearOffset() const {
    if (mRenderingIndex < 0) {
        return 0.0f;
    }

    return mRenderingIndex == 0 ? getCurrentParam().getMirror1NearOffset() :
                                  getCurrentParam().getMirror2NearOffset();
}

/**
 * Gets the mirror camera being rendered.
 * @return Mirror camera, or nullptr if no mirror is rendered.
 */
const sead::LookAtCamera* ShaderMirrorDirector::getRenderingCamera() const {
    if (mRenderingIndex < 0) {
        return nullptr;
    }

    return &mCameras[mRenderingIndex];
}

/**
 * Hides the rendered mirror actor itself before drawing the mirror view, if requested.
 */
void ShaderMirrorDirector::startRendering() {
    MirrorActorBase* actor = getRenderingActor();

    if (!actor->isHideAtSelfMirror()) {
        return;
    }

    mIsHideModel[mRenderingIndex] = isHideModel(actor);

    if (mIsHideModel[mRenderingIndex]) {
        return;
    }

    hideModelIfShow(actor);
    mLiveActorKit->getExecuteDirector()->mRequestKeeper->executeRequestActorDrawAllOn();
    mLiveActorKit->getExecuteDirector()->mRequestKeeper->executeRequestActorDrawAllOff();
}

/**
 * Blurs the mirror view and shows the rendered mirror actor again.
 * @param shaderMode Current shader mode.
 * @return Shader mode after rendering.
 */
agl::ShaderMode ShaderMirrorDirector::endRendering(agl::ShaderMode shaderMode) {
    mColorTarget.invalidateGPUCache(GameFrameworkNx::getAglDrawContext());

    if (getCurrentParam().getBlurType() != -1) {
        sead::GraphicsContext graphicsContext;
        graphicsContext.setBlendEnable(false);
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.apply(GameFrameworkNx::getAglDrawContext());

        s32 width = mRenderBufferSize.x;
        s32 height = mRenderBufferSize.y;
        agl::utl::DynamicTextureAllocator* allocator =
            agl::utl::DynamicTextureAllocator::instance();
        agl::TextureData* workTexture = allocator->alloc(
            GameFrameworkNx::getAglDrawContext(), "For Blur",
            agl::TextureFormat::cTextureFormat_R11_G11_B10_float, width, height, 1, nullptr,
            agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);
        s32 blurCount = getCurrentParam().getBlurCount();

        for (s32 i = 0; i < blurCount; i++) {
            blurTexture(mColorTexture, workTexture, width, height,
                        static_cast<agl::utl::ImageFilter2D::GaussianKernel>(
                            getCurrentParam().getBlurType()));
        }

        allocator->free(workTexture);
        graphicsContext.setBlendEnable(false);
        graphicsContext.setDepthEnable(true, true);
        graphicsContext.apply(GameFrameworkNx::getAglDrawContext());
        shaderMode = agl::cShaderMode_UniformBlock;
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), shaderMode);
    }

    MirrorActorBase* actor = getRenderingActor();

    if (actor->isHideAtSelfMirror() && !mIsHideModel[mRenderingIndex]) {
        showModelIfHide(actor);
        mLiveActorKit->getExecuteDirector()->mRequestKeeper->executeRequestActorDrawAllOn();
        mLiveActorKit->getExecuteDirector()->mRequestKeeper->executeRequestActorDrawAllOff();
    }

    return shaderMode;
}

/**
 * Gets the mirror camera of a mirror actor.
 * @param pActor Mirror actor.
 * @return Mirror camera of the actor.
 */
const sead::LookAtCamera* ShaderMirrorDirector::getMirrorCamera(const LiveActor* pActor) const {
    return mMirrorActors[1] == pActor ? &mCameras[1] : &mCameras[0];
}

/**
 * Updates the requested parameters, the buffer size, the mirror cameras and the mirror actors,
 * and alternates the rendered mirror when two mirrors are active.
 */
void ShaderMirrorDirector::movement() {
    updateRequest();

    f32 scale =
        mGraphicsSystemInfo->getGraphicsStressDirector()->isFullResolution() ? 1.5f : 1.0f;
    mRenderBufferSize.set(scale * (getCurrentParam().getMirrorTexWidthScale() * 8),
                          scale * (getCurrentParam().getMirrorTexHeightScale() * 8));

    updateMirrorCamera(&mCameras[0], mMirrorActors[0], getCurrentParam().getMirror1ActorOffsetY());
    updateMirrorCamera(&mCameras[1], mMirrorActors[1], getCurrentParam().getMirror2ActorOffsetY());

    mMirrorActors[0] =
        static_cast<MirrorActorBase*>(findMirrorActor(getCurrentParam().getMirror1ActorName()));
    mMirrorActors[1] =
        static_cast<MirrorActorBase*>(findMirrorActor(getCurrentParam().getMirror2ActorName()));

    if (mMirrorActors[1] != nullptr) {
        mRenderingIndex = modi(mRenderingIndex + 3, 2);
    } else {
        mRenderingIndex = 0;
    }
}

/**
 * Creates a mirror actor.
 * @param pName Actor name.
 */
MirrorActorBase::MirrorActorBase(const char* pName) : LiveActor(pName) {}

/**
 * Reads whether the actor hides itself in its own mirror view.
 * @param rInfo Actor init info.
 */
void MirrorActorBase::initByArg(const ActorInitInfo& rInfo) {
    tryGetArg(&mIsHideAtSelfMirror, rInfo, "IsHideAtSelfMirror");
}

/**
 * Makes the materials programmable so the mirror matrix can be set.
 */
void MirrorActorBase::initAfterPlacement() {
    setMaterialProgrammable(this);
}

/**
 * Sets the view-projection matrix of the actor's mirror camera to all materials and selects the
 * mirror texture.
 */
void MirrorActorBase::control() {
    ShaderMirrorDirector* director = MirrorFunction::getShaderMirrorDirector(this);
    const sead::LookAtCamera* camera = director->getMirrorCamera(this);
    sead::Matrix44f viewProjMtx;
    viewProjMtx.setMul(getProjectionMtxSub(this), camera->getMatrix());
    nn::g3d::ModelObj* modelObj = mModelKeeper->getModelCafe()->getModelG3D()->getModelObj();

    s32 materialNum = modelObj->GetNumMaterials();

    for (s32 i = 0; i < materialNum; i++) {
        nn::g3d::MaterialObj* material = modelObj->GetMaterial(i);
        s32 index = material->GetResource()->FindShaderParamIndex("uMirrorViewProj");

        if (index != -1) {
            copyMatrix(material->EditShaderParam<sead::Matrix44f>(index), viewProjMtx);
        }
    }

    setEnvTextureMirror(this, director->tryFindMirrorTextureId(this));
}

}  // namespace al

/**
 * Gets the mirror director of an actor's scene.
 * @param pActor Actor of the scene.
 * @return Mirror director.
 */
al::ShaderMirrorDirector* MirrorFunction::getShaderMirrorDirector(const al::LiveActor* pActor) {
    return pActor->getSceneInfo()->graphicsSystemInfo->mShaderMirrorDirector;
}
