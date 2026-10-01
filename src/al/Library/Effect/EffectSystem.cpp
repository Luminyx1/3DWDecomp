#include "Library/Effect/EffectSystem.hpp"

#include <agl/common/aglDrawContext.h>
#include <agl/driver/aglNVNMgr.h>
#include <agl/shadow/aglDepthShadow.h>
#include <heap/seadHeapMgr.h>
#include <nn/gfx/gfx_Sampler.h>
#include <nn/gfx/gfx_Texture.h>
#include <ptcl/seadPtclConfig.h>

#include "Library/Camera/CameraDirector.hpp"
#include "Library/Effect/EffectShaderHolderNew.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/EffectCameraHolder.hpp"
#include "Project/Effect/EffectDataBase.hpp"
#include "Project/Effect/EffectEnvParam.hpp"
#include "Project/Effect/EffectGroupDrawer.hpp"
#include "Project/Effect/EffectHeap.hpp"
#include "Project/Effect/EffectLayoutDrawer.hpp"

namespace al {

using EffectSystemFunctor = FunctorV0M<EffectSystem*, void (EffectSystem::*)()>;

static inline f32 getFrameRate(const nn::vfx::EmitterSet* pEmitterSet) {
    if (*reinterpret_cast<const u8*>(&pEmitterSet->m_IsCalcEnable) == 1) {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Creates the effect system with the default effect archives.
 * @param pDrawContext Draw context used for rendering.
 * @param pHeap Heap to create the system in.
 * @return The created effect system.
 */
EffectSystem* EffectSystem::createSystem(agl::DrawContext* pDrawContext, sead::Heap* pHeap) {
    sead::ScopedCurrentHeapSetter setter(pHeap);
    EffectSystem* effectSystem = new EffectSystem();
    effectSystem->mHeap = pHeap;
    effectSystem->setDrawContext(pDrawContext);
    effectSystem->addResourcePath("EffectData/AllNewEffectData");
    effectSystem->addResourcePath("EffectData/AllEffectData");
    effectSystem->addResourcePath("EffectData/AllPart2EffectData");
    effectSystem->init();
    return effectSystem;
}

/**
 * Creates the effect system with the default effect archives plus the patch archive.
 * @param pDrawContext Draw context used for rendering.
 * @param pHeap Heap to create the system in.
 * @return The created effect system.
 */
EffectSystem* EffectSystem::createSystemWithPatchResouce(agl::DrawContext* pDrawContext,
                                                         sead::Heap* pHeap) {
    sead::ScopedCurrentHeapSetter setter(pHeap);
    EffectSystem* effectSystem = new EffectSystem();
    effectSystem->mHeap = pHeap;
    effectSystem->setDrawContext(pDrawContext);
    effectSystem->addResourcePath("EffectData/AllPatchEffectData");
    effectSystem->addResourcePath("EffectData/AllNewEffectData");
    effectSystem->addResourcePath("EffectData/AllEffectData");
    effectSystem->addResourcePath("EffectData/AllPart2EffectData");
    effectSystem->init();
    return effectSystem;
}

/**
 * Loads the particle resources of an effect system into its own heap.
 * @param pEffectSystem Effect system to load the resources of.
 */
void EffectSystem::loadEffectResource(EffectSystem* pEffectSystem) {
    pEffectSystem->loadPtclResource(pEffectSystem->mHeap);
}

/**
 * Enters the particle resources of all registered archives into the particle system.
 * @param pHeap Heap to load the resources into.
 */
void EffectSystem::loadPtclResource(sead::Heap* pHeap) {
    mIsLoadedPtclResource = true;
    sead::ScopedCurrentHeapSetter setter(pHeap);

    mResourceIndex = 0;
    for (s32 i = 0; i < mResourcePathNum; i++) {
        Resource* resource = findOrCreateResource(mResourcePaths[i], nullptr);
        PtclSystem* ptclSystem = mEffectSystemInfo.mPtclSystem;
        ptclSystem->entryResource(pHeap, resource->getOtherFile("EffectData.ptcl", nullptr),
                                  mResourceIndex);
        mResourceIndex++;
    }

    mEffectSystemInfo.mPtclSystem->entryResourceEnd();
}

/**
 * Creates the effect system and loads its particle resources unless loading is delayed.
 * @param pDrawContext Draw context used for rendering.
 * @param pHeap Heap to create the system in.
 * @param isDelayLoadResource Whether loading the particle resources is left for later.
 * @return The created effect system.
 */
EffectSystem* EffectSystem::initializeSystem(agl::DrawContext* pDrawContext, sead::Heap* pHeap,
                                             bool isDelayLoadResource) {
    EffectSystem* effectSystem = createSystem(pDrawContext, pHeap);

    if (!isDelayLoadResource) {
        loadEffectResource(effectSystem);
    }

    return effectSystem;
}

/**
 * Creates the effect system with the patch archive and loads its particle resources unless
 * loading is delayed.
 * @param pDrawContext Draw context used for rendering.
 * @param pHeap Heap to create the system in.
 * @param isDelayLoadResource Whether loading the particle resources is left for later.
 * @return The created effect system.
 */
EffectSystem* EffectSystem::initializeSystemWithPatchResource(agl::DrawContext* pDrawContext,
                                                              sead::Heap* pHeap,
                                                              bool isDelayLoadResource) {
    EffectSystem* effectSystem = createSystemWithPatchResouce(pDrawContext, pHeap);

    if (!isDelayLoadResource) {
        loadEffectResource(effectSystem);
    }

    return effectSystem;
}

/**
 * Returns whether compute shader emitters are calculated in a batch.
 * @return Always true.
 */
bool EffectSystem::isEnableBatchCompute() {
    return true;
}

/**
 * Returns the number of frames calculated while the game is paused.
 * @return Always 1.
 */
s32 EffectSystem::getPauseForceCalcFrame() {
    return 1;
}

/**
 * Constructs an empty effect system.
 */
EffectSystem::EffectSystem()
    : mHeap(nullptr), mEffectCameraHolder(nullptr), mGroupDrawerNum(0), mGroupDrawers(nullptr),
      mResourcePathNum(0), mResourceIndex(0), mIsLoadedPtclResource(false), mIsStopCalc(false),
      mIsStopCalcByDemo(false), mIsSwapBuffer(false), mCalcEffectNum(0), mLayoutDrawerNum(0),
      mLayoutDrawers(nullptr), mShaderHolder(nullptr), _398(nullptr), _3a0(nullptr),
      _3a8(nullptr), mEffectHeap(nullptr), mDrawContext(nullptr), mEffectEnvParam(nullptr),
      mCalculateFlag(0) {
    mEffectCameraHolder = new EffectCameraHolder();

    for (s32 i = 0; i < 6; i++) {
        mResourcePaths[i] = nullptr;
    }

    mEffectEnvParam = new EffectEnvParam();
}

/**
 * Sets the draw context used for rendering.
 * @param pDrawContext Draw context.
 */
void EffectSystem::setDrawContext(agl::DrawContext* pDrawContext) {
    mDrawContext = pDrawContext;
}

/**
 * Adds an effect archive to load the particle resources from.
 * @param pPath Archive path.
 */
void EffectSystem::addResourcePath(const char* pPath) {
    mResourcePaths[mResourcePathNum] = pPath;
    mResourcePathNum++;
}

void EffectSystem::init() {
    mEffectHeap = EffectHeap::create(0x1600000, "GPUパーティクル用ヒープ");
    EffectHeapForNw* systemHeap = new EffectHeapForNw(getCurrentHeap());
    EffectHeapForNw* dynamicHeap = new EffectHeapForNw(mEffectHeap);

    sead::ptcl::Config config;
    config.SetDynamicHeap(dynamicHeap);
    config.SetSystemHeap(systemHeap);
    config.setSystemHeap(getCurrentHeap());
    config.SetTemporaryBufferSize(0xa00000);
    config.SetParticleSortBufferNum(0x800);
    config.SetResourceNum(mResourcePathNum);
    config.SetGpuBufferSize(0xc0000);
    config.SetEmitterSetNum(0x1000);
    config.SetEmitterNum(0x1000);
    config.SetStripeNum(0x80);
    config.setGpuHeap(mEffectHeap);
    config.SetEnableGpuBufferFixed(true);
    config.setResourceNum(mResourcePathNum);
    config.SetGfxDevice(agl::driver::NVNMgr::instance()->getGfxDevice());
    config.SetMultiBufferNum(3);

    PtclSystem* ptclSystem = new PtclSystem(config, this);
    mEffectSystemInfo._18 = mResourcePathNum;
    mEffectSystemInfo.mPtclSystem = ptclSystem;

    ptclSystem->setRegisterTextureViewSlot(
        [](nn::gfx::DescriptorSlot* pSlot, const nn::gfx::TextureView& rTextureView,
           void* pUserData) -> bool {
            u32 id = agl::driver::NVNMgr::instance()->registerTexture(
                static_cast<const NVNtexture*>(rTextureView.ToData()->pNvnTexture.ptr),
                static_cast<const NVNtextureView*>(rTextureView.ToData()->pNvnTextureView.ptr),
                "effect");
            pSlot->ToData()->value = id;
            return id != 0;
        });
    ptclSystem->setRegisterSamplerSlot(
        [](nn::gfx::DescriptorSlot* pSlot, const nn::gfx::Sampler& rSampler,
           void* pUserData) -> bool {
            u32 id = agl::driver::NVNMgr::instance()->registerSampler(
                static_cast<const NVNsampler*>(rSampler.ToData()->pNvnSampler.ptr), "effect");
            pSlot->ToData()->value = id;
            return id != 0;
        });
    ptclSystem->setUnregisterTextureViewSlot([](nn::gfx::DescriptorSlot* pSlot,
                                                const nn::gfx::TextureView& rTextureView,
                                                void* pUserData) {
        agl::driver::NVNMgr::instance()->releaseTexture(pSlot->ToData()->value);
    });
    ptclSystem->setUnregisterSamplerSlot(
        [](nn::gfx::DescriptorSlot* pSlot, const nn::gfx::Sampler& rSampler, void* pUserData) {
            agl::driver::NVNMgr::instance()->releaseSampler(pSlot->ToData()->value);
        });

    ptclSystem->RegisterTextureViewToDescriptorPool(ptclSystem->getRegisterTextureViewSlot(),
                                                    nullptr);
    mEffectSystemInfo.mPtclSystem->RegisterSamplerToDescriptorPool(
        mEffectSystemInfo.mPtclSystem->getRegisterSamplerSlot(), nullptr);

    mShaderHolder = new EffectShaderHolderNew(mEffectSystemInfo.mPtclSystem, mDrawContext,
                                              mEffectEnvParam);
    loadDbResource(nullptr);
}

const EffectDrawCategoryInfo& EffectSystem::getDrawCategory(s32 index) const {
    return mEffectSystemInfo.mEffectDataBase->getDrawCategory(index);
}

/**
 * Loads the effect database and creates the group and layout drawers.
 * @param pHeap Unused.
 */
void EffectSystem::loadDbResource(sead::Heap* pHeap) {
    EffectDataBase* dataBase = new EffectDataBase("EffectData/EffectDataBase");
    mEffectSystemInfo.mEffectDataBase = dataBase;
    mGroupDrawerNum = dataBase->getDrawCategoryNum();
    mGroupDrawers = new EffectGroupDrawer*[mGroupDrawerNum];

    for (s32 i = 0; i < mGroupDrawerNum; i++) {
        mGroupDrawers[i] = new EffectGroupDrawer(this, getDrawCategory(i).mName, i,
                                                 getDrawCategory(i).mIsEnableZSort,
                                                 getDrawCategory(i).mIsAlwaysUpdateUbo,
                                                 getDrawCategory(i).mIsScreenEffect);
    }

    mLayoutDrawerNum = 3;
    mLayoutDrawers = new EffectLayoutDrawer*[mLayoutDrawerNum];
    mLayoutDrawers[0] = new EffectLayoutDrawer(this, "２Ｄエフェクト", mGroupDrawerNum - 3, 2);
    mLayoutDrawers[1] = new EffectLayoutDrawer(this, "２Ｄベースエフェクト", mGroupDrawerNum - 2, 2);
    mLayoutDrawers[2] = new EffectLayoutDrawer(this, "2DEffectAboveBlur", mGroupDrawerNum - 1, 2);
}

/**
 * Starts a new particle frame.
 */
void EffectSystem::initScene() {
    mEffectSystemInfo.mPtclSystem->BeginFrame();
}

/**
 * Finishes the scene initialization.
 */
void EffectSystem::endInit() {}

/**
 * Registers the effect executors and drawers to a scene's execute director.
 * @param pExecuteDirector Execute director of the scene.
 */
void EffectSystem::startScene(ExecuteDirector* pExecuteDirector) {
    registerExecutorFunctor("エフェクト（前処理）", pExecuteDirector,
                            EffectSystemFunctor(this, &EffectSystem::preprocess));
    registerExecutorFunctor("エフェクト（後処理）", pExecuteDirector,
                            EffectSystemFunctor(this, &EffectSystem::postprocess));

    for (s32 i = 0; i < mGroupDrawerNum; i++) {
        registerExecutorUser(mGroupDrawers[i], pExecuteDirector, mGroupDrawers[i]->getName());
    }

    for (s32 i = 0; i < mLayoutDrawerNum; i++) {
        registerExecutorUser(mLayoutDrawers[i], pExecuteDirector, mLayoutDrawers[i]->getName());
    }

    mEffectSystemInfo._0 = 0;
}

/**
 * Begins the particle frame and swaps the particle buffers if the frame is drawn.
 */
void EffectSystem::preprocess() {
    mCalculateFlag = 0;
    mIsSwapBuffer = GameFrameworkNx::sInstance->_27c || !GameFrameworkNx::sInstance->_27b;
    mEffectSystemInfo.mPtclSystem->BeginFrame();

    if (mIsSwapBuffer) {
        mEffectSystemInfo.mPtclSystem->SwapBuffer();
        mShaderHolder->swapUbo();
    }
}

inline void EffectSystem::calcParticleImpl(u64 userData) {
    for (s32 i = 0; i < mGroupDrawerNum; i++) {
        if ((mCalculateFlag & (1 << i)) != 0) {
            continue;
        }

        nn::vfx::EmitterSet* next = mEffectSystemInfo.mPtclSystem->GetEmitterSetHead(i);
        while (next != nullptr) {
            nn::vfx::EmitterSet* emitterSet = next;
            next = emitterSet->GetNext();

            if (emitterSet->IsAlive() && emitterSet->GetUserData() == userData) {
                mEffectSystemInfo.mPtclSystem->Calculate(
                    emitterSet, getFrameRate(emitterSet),
                    static_cast<nn::vfx::BufferSwapMode>(mIsSwapBuffer << 1));
            }
        }
    }
}

/**
 * Calculates the emitter sets registered with addCalcEffect and advances the frame counter.
 */
void EffectSystem::postprocess() {
    for (s64 i = 0; i < mCalcEffectNum; i++) {
        calcParticleImpl(mCalcEffects[i]);
    }

    mCalcEffectNum = 0;
    mEffectSystemInfo._0++;
}

/**
 * Detaches the graphics system info and kills all emitter sets.
 */
void EffectSystem::endScene() {
    mShaderHolder->setGraphicsSystemInfo(nullptr);
    mEffectSystemInfo.mPtclSystem->KillAllEmitterSet();
    mEffectSystemInfo._0 = 0;
}

/**
 * Passes the scene camera info of a camera director to the effect camera holder.
 * @param pCameraDirector Camera director, may be null.
 */
void EffectSystem::setCameraDirector(CameraDirector* pCameraDirector) {
    if (pCameraDirector != nullptr) {
        mEffectCameraHolder->setSceneCameraInfo(pCameraDirector->getSceneCameraInfo());
    }
}

/**
 * Calculates the alive emitter sets with the given user data.
 * @param userData User data of the emitter sets to calculate.
 */
void EffectSystem::calcParticle(u64 userData) {
    calcParticleImpl(userData);
}

/**
 * Sets the graphics system info used by the shader holder.
 * @param pGraphicsSystemInfo Graphics system info.
 */
void EffectSystem::setGraphicsSystemInfo(const GraphicsSystemInfo* pGraphicsSystemInfo) {
    mShaderHolder->setGraphicsSystemInfo(pGraphicsSystemInfo);
}

/**
 * Executes the group drawer with the given name.
 * @param pGroupName Name of the group.
 */
void EffectSystem::updateEffect(const char* pGroupName) const {
    s32 i = 0;
    while (!isEqualString(mGroupDrawers[i]->getName(), pGroupName)) {
        i++;
    }

    mGroupDrawers[i]->execute();
}

/**
 * Finds the group drawer with the given name.
 * @param pGroupName Name of the group.
 * @return The group drawer, or null if not found.
 */
EffectGroupDrawer* EffectSystem::findGroupDrawer(const char* pGroupName) const {
    for (s32 i = 0; i < mGroupDrawerNum; i++) {
        if (isEqualString(mGroupDrawers[i]->getName(), pGroupName)) {
            return mGroupDrawers[i];
        }
    }

    return nullptr;
}

/**
 * Calculates all compute shader emitters in a batch.
 */
void EffectSystem::calcEffectCompute() const {
    nn::gfx::CommandBuffer* commandBuffer = mDrawContext->getCommandBuffer();
    mEffectSystemInfo.mPtclSystem->BatchCalculationComputeShaderEmitter(commandBuffer, nullptr,
                                                                        0xffffffff);
}

/**
 * Draws the effects of a group with an explicit camera position.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param rCamPos Camera position.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param pGroupName Name of the group.
 * @param renderPath Render path flags.
 * @param isCalcCompute Whether compute shader emitters are calculated while drawing.
 */
void EffectSystem::drawEffectWithRenderPathAndCamPos(const sead::Matrix44f& rProjMtx,
                                                     const sead::Matrix34f& rViewMtx,
                                                     const sead::Vector3f& rCamPos, f32 near,
                                                     f32 far, f32 fovy, const char* pGroupName,
                                                     u32 renderPath, bool isCalcCompute) const {
    findGroupDrawer(pGroupName)
        ->drawEffectWithRenderPathAndCamPos(mDrawContext, rProjMtx, rViewMtx, rCamPos, near, far,
                                            fovy, renderPath, isCalcCompute);
}

/**
 * Draws the effects of a group.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param pGroupName Name of the group.
 * @param renderPath Render path flags.
 * @param isCalcCompute Whether compute shader emitters are calculated while drawing.
 */
void EffectSystem::drawEffectWithRenderPath(const sead::Matrix44f& rProjMtx,
                                            const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                                            f32 fovy, const char* pGroupName, u32 renderPath,
                                            bool isCalcCompute) const {
    EffectGroupDrawer* drawer = findGroupDrawer(pGroupName);

    if (drawer != nullptr) {
        drawer->drawEffectWithRenderPath(mDrawContext, rProjMtx, rViewMtx, near, far, fovy,
                                         renderPath, isCalcCompute);
    }
}

/**
 * Updates the clip volume of a depth shadow with the effects of a group.
 * @param pDepthShadow Depth shadow.
 * @param pGroupName Name of the group.
 * @param renderPath Render path flags.
 */
void EffectSystem::calcShadowClipVolume(agl::sdw::DepthShadow* pDepthShadow,
                                        const char* pGroupName, u32 renderPath) const {
    findGroupDrawer(pGroupName)->calcShadowClipVolume(pDepthShadow, renderPath);
}

/**
 * Registers emitter sets with the given user data to be calculated in postprocess.
 * @param userData User data of the emitter sets.
 */
void EffectSystem::addCalcEffect(u64 userData) {
    if (mCalcEffectNum < 0x60) {
        mCalcEffects[mCalcEffectNum++] = userData;
    }
}

/**
 * Checks whether any group has emitters rendered with the given draw path flags.
 * @param flag Draw path flags.
 * @return Whether a rendering emitter was found.
 */
bool EffectSystem::isHasRenderingEmitter(u32 flag) const {
    for (s32 i = 0; i < mGroupDrawerNum; i++) {
        s32 groupId = mGroupDrawers[i]->getGroupId();

        for (s32 j = 0; j < mEffectSystemInfo.mPtclSystem->GetRenderingInfoNum(); j++) {
            const nn::vfx::RenderingInfo& info = mEffectSystemInfo.mPtclSystem->GetRenderingInfo(j);

            if ((info.m_RenderingEmitterFlag[groupId] & flag) != 0) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Marks a group as calculated in this frame.
 * @param groupId Group ID.
 */
void EffectSystem::checkCalculateFlag(s32 groupId) {
    if (static_cast<u32>(groupId) < 32) {
        mCalculateFlag |= 1 << groupId;
    }
}

/**
 * Does nothing.
 * @param groupId Group ID.
 */
void EffectSystem::calcParticle(s32 groupId) {}

/**
 * Does nothing.
 * @param groupId Group ID.
 */
void EffectSystem::calcChildParticle(s32 groupId) {}

}  // namespace al

/**
 * Destroys the heap wrapper.
 */
EffectHeapForNw::~EffectHeapForNw() = default;

/**
 * Allocates memory from the wrapped heap.
 * @param size Size in bytes.
 * @param alignment Alignment in bytes.
 * @return The allocated memory.
 */
void* EffectHeapForNw::Alloc(size_t size, size_t alignment) {
    return mHeap->tryAlloc(size, alignment);
}

/**
 * Frees memory allocated from the wrapped heap.
 * @param ptr Memory to free.
 */
void EffectHeapForNw::Free(void* ptr) {
    mHeap->free(ptr);
}
