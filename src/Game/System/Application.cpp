#include "System/Application.hpp"
#include "Player/Normal/PlayerAmiiboDirector.hpp"
#include "System/AssetLoadingThread.hpp"
#include "System/GameDataConst.hpp"
#include "System/Main.hpp"
#include "System/RootTask.hpp"
#include "al/Library/Debug/Host.hpp"
#include "al/Library/Framework/GameFrameworkNx.hpp"
#include "al/Library/Memory/HeapUtil.hpp"
#include "al/Library/Message/LanguageUtil.hpp"
#include "al/Library/Resource/ResourceFunction.hpp"
#include "al/Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "al/Library/System/GameSystemInfo.hpp"
#include "al/Library/System/SystemKit.hpp"
#include "al/Project/Base/StringUtil.hpp"
#include "al/Project/Memory/MemorySystem.hpp"
#include <framework/seadFramework.h>
#include <framework/seadTaskMgr.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <nn/account.h>
#include <nn/oe.h>
#include <prim/seadEnvUtil.h>
#include <prim/seadSafeString.h>
#include <thread/seadThread.h>

/**
 * @brief Priorities of the worker threads started by the engine systems.
 */
struct SystemThreadPriority {
    /**
     * @brief Derive every system thread priority from sead's default priority.
     */
    SystemThreadPriority() {
        mMain = sead::Thread::cDefaultPriority;
        mFileLoader = sead::Thread::cDefaultPriority + 1;
        mSaveData = sead::Thread::cDefaultPriority + 4;
        mResource = sead::Thread::cDefaultPriority + 2;
    }

    s32 mFileLoader;
    s32 mResource;
    s32 mSaveData;
    s32 mMain;
};

/**
 * @brief Host and thread settings used while the application creates its systems.
 */
struct SystemSetting {
    SystemThreadPriority mThreadPriority;
    sead::FixedSafeString<256> mUserName;
};

namespace {

SystemSetting sSystemSetting;

/**
 * @brief Fill the user name from the host's computer name, or the account name as a fallback.
 */
void initUserName() {
    if (sead::EnvUtil::getEnvironmentVariable(&sSystemSetting.mUserName, "COMPUTERNAME") == -1) {
        al::getUserName(&sSystemSetting.mUserName);
    }
}

}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(Application)

/**
 * @brief Create the engine systems, heaps, framework and stationed resources.
 * @param argc Host argument count (unused).
 * @param ppArgv Host argument vector (unused).
 */
void Application::init(int argc, char** ppArgv) {
    nn::account::Initialize();
    nn::oe::Initialize();
    al::setCpuBoost(true, true);

    sead::Heap* pRootHeap = nullptr;
    if (sead::HeapMgr::getRootHeapNum() != 0) {
        pRootHeap = sead::HeapMgr::getRootHeap(0);
    }

    {
        sead::ScopedCurrentHeapSetter rootHeapSetter(pRootHeap);
        mpSystemKit = new al::SystemKit();
        mpSystemKit->createMemorySystem(pRootHeap, 0, 0, 0);
    }

    sead::Heap* pStationedHeap = mpSystemKit->getMemorySystem()->getStationedHeap();
    sead::ScopedCurrentHeapSetter stationedHeapSetter(pStationedHeap);

    sead::ExpHeap* pGraphicsHeap = sead::ExpHeap::create(
        0, "GraphicsHeap", pStationedHeap, 8, sead::Heap::cHeapDirection_Forward, false);
    al::addNamedHeap(pGraphicsHeap, nullptr);

    {
        sead::ScopedCurrentHeapSetter graphicsHeapSetter(pGraphicsHeap);

        sead::GameFrameworkNx::CreateArg createArg;
        createArg.command_memory_size = 0x80000;
        createArg.control_memory_size = 0x80000;
        createArg.shader_scratch_memory_scale = 0x1000;
        createArg.vblank_wait_interval = 1;
        createArg.is_triple_buffer = true;
        createArg.is_debug = false;
        createArg.is_apply_deferred_finalizes = false;
        createArg.queue_compute_memory_size = -1;
        createArg.queue_command_memory_size = 0x20000;
        createArg.nvn_debug_level = 0;
        createArg.texture_descriptor_num = 0x4000;
        createArg.present_thread_priority = 2;
        createArg.graphics_memory_size = 0x900000;
        createArg.graphics_devtools_memory_size = 0;
        createArg.clear_color = sead::Color4f::cBlack;
        createArg.create_method_frame_buffer = false;
        createArg.queue_control_memory_size = 0x40000;
        createArg.display_width = 1920;
        createArg.display_height = 1080;

        al::GameFrameworkNx* pFramework = new al::GameFrameworkNx(createArg);
        mpFramework = pFramework;

        pFramework->initializeGraphicsSystem(sead::HeapMgr::instance()->getCurrentHeap(),
                                             sead::Vector2f(1920.0f, 1080.0f));

        al::GameFrameworkNx::AglInitArg aglInitArg;
        aglInitArg.heap = sead::HeapMgr::instance()->getCurrentHeap();
        aglInitArg.virtualWidth = 1920;
        aglInitArg.virtualHeight = 1080;
        aglInitArg.dockedWidth = 1920;
        aglInitArg.dockedHeight = 1080;
        aglInitArg.handheldWidth = 1280;
        aglInitArg.handheldHeight = 720;
        aglInitArg.aglHeapSize = 0xb400000;
        aglInitArg.workHeapSize = 0x4000000;
        aglInitArg.minGPUMemBlockSize = 0x40000;
        aglInitArg.dynamicTextureSize = 0x6400000;
        static_cast<al::GameFrameworkNx*>(mpFramework)->initAgl(aglInitArg);

        mpSystemKit->mFramework = mpFramework;
        pGraphicsHeap->adjust();

        initUserName();

        al::SystemKit* pSystemKit = mpSystemKit;
        pSystemKit->_28 = sSystemSetting.mUserName.cstr();
    }

    const u32 audioStationedSize = 0x9b00000;
    sead::ExpHeap* pAudioHeap =
        sead::ExpHeap::create(audioStationedSize + 0xe00000, "AudioHeap", nullptr, 8,
                              sead::Heap::cHeapDirection_Forward, false);
    al::addNamedHeap(pAudioHeap, nullptr);
    al::addNamedHeap(sead::ExpHeap::create(audioStationedSize, "AudioStationedResourceHeap",
                                           pAudioHeap, 8, sead::Heap::cHeapDirection_Forward,
                                           false),
                     nullptr);
    al::addNamedHeap(sead::ExpHeap::create(0x500000, "AudioSubHeap", pAudioHeap, 8,
                                           sead::Heap::cHeapDirection_Forward, false),
                     nullptr);

    al::initRegionAndLanguage();
    const char* pLanguage = al::getLanguageString();

    if (!al::isEqualString(pLanguage, "UsEn") && !al::isEqualString(pLanguage, "UsFr") &&
        !al::isEqualString(pLanguage, "UsEs") && !al::isEqualString(pLanguage, "EuEn") &&
        !al::isEqualString(pLanguage, "EuIt") && !al::isEqualString(pLanguage, "EuFr") &&
        !al::isEqualString(pLanguage, "EuDe") && !al::isEqualString(pLanguage, "EuNl") &&
        !al::isEqualString(pLanguage, "EuEs") && !al::isEqualString(pLanguage, "EuPt") &&
        !al::isEqualString(pLanguage, "EuRu") && !al::isEqualString(pLanguage, "JpJa") &&
        !al::isEqualString(pLanguage, "CNzh") && !al::isEqualString(pLanguage, "TWzh") &&
        !al::isEqualString(pLanguage, "KRko")) {
        al::forceInitLanguage(alLanguage_USen);
    }

    mpSystemKit->createFileLoader(sSystemSetting.mThreadPriority.mFileLoader, false);
    mpSystemKit->createResourceSystem(nullptr, sSystemSetting.mThreadPriority.mResource, 0x400000,
                                      true);

    {
        sead::ExpHeap* pSystemHeap =
            sead::ExpHeap::create(0, "StationedSystemHeap", pStationedHeap, 8,
                                  sead::Heap::cHeapDirection_Forward, false);
        al::addNamedHeap(pSystemHeap, nullptr);

        sead::ScopedCurrentHeapSetter systemHeapSetter(pSystemHeap);

        al::addResourceCategory("Stationed[System]", 0x20, pSystemHeap);
        al::createCategoryResourceAll("Stationed[System]", nullptr);
        sead::Heap* pAudioStationedHeap = al::findNamedHeap("AudioStationedResourceHeap");
        al::addResourceCategory("常駐[オーディオ]", 6, pAudioStationedHeap);
        al::addResourceCategory("Stationed[Shader]", 0x25, pSystemHeap);
        al::createCategoryResourceAll("Stationed[Shader]", nullptr);

        sead::ExpHeap* pFontHeap =
            sead::ExpHeap::create(0, "StationedFontHeap", pSystemHeap, 8,
                                  sead::Heap::cHeapDirection_Forward, false);
        al::addNamedHeap(pFontHeap, nullptr);
        al::addResourceCategory("Stationed[Localize]", 0x10, pFontHeap);
        al::createCategoryResourceAll("Stationed[Localize]", nullptr);
        pFontHeap->adjust();

        al::addResourceCategory("Stationed[PreLoad]", 0x80, pSystemHeap);
        al::createCategoryResourceAll("Stationed[PreLoad]", nullptr);
        pSystemHeap->adjust();

        sead::ExpHeap* pResourceHeap =
            sead::ExpHeap::create(0x40500000, "StationedResourceHeap", pStationedHeap, 8,
                                  sead::Heap::cHeapDirection_Forward, true);
        al::addNamedHeap(pResourceHeap, nullptr);

        {
            sead::ScopedCurrentHeapSetter assetHeapSetter(pStationedHeap);
                rc::AssetLoadingThread::createInstance(pResourceHeap);
        }
    }

    const s32 ghostSizeMax = static_cast<s32>(GameDataConst::getSaveDataSizeGhostMax());
    const u32 saveDataWorkSize =
        ghostSizeMax > 0x10000 ? GameDataConst::getSaveDataSizeGhostMax() : 0x10000;
    mpSystemKit->createSaveDataSystem(saveDataWorkSize, sSystemSetting.mThreadPriority.mSaveData);

    al::ShaderHolder::createInstance(nullptr);
    al::ShaderHolder::instance()->initAndLoadAllFromDir("ShaderData", pStationedHeap);
    PlayerAmiiboDirector::initRandomSeed();
}

/**
 * @brief Run the framework's main loop with the root task in the stationed heap.
 */
void Application::run() {
    sead::TaskBase::CreateArg createArg(&sead::TTaskFactory<RootTask>);
    sead::HeapPolicies& rPolicies = createArg.heap_policies;
    rPolicies.mPolicies[rPolicies.mPrimaryIndex].parent =
        mpSystemKit->getMemorySystem()->getStationedHeap();
    rPolicies.mPolicies[rPolicies.mPrimaryIndex].adjust = true;

    sead::Framework* pFramework = mpFramework;
    sead::Heap* pStationedHeap = mpSystemKit->getMemorySystem()->getStationedHeap();
    pFramework->run(pStationedHeap, createArg, sead::Framework::RunArg());
}

/**
 * @brief Access the framework's root game task.
 * @return The root task; requires an initialized framework.
 */
RootTask* Application::getRootTask() const {
    return static_cast<RootTask*>(mpFramework->mTaskMgr->mRootTask);
}

/**
 * @brief Handle the pre-swap callback, which performs no work in this build.
 */
void Application::preSwapBufferCallback() {}

/**
 * @brief Create an application before framework initialization.
 */
Application::Application() : mpSystemKit(nullptr), mpFramework(nullptr) {}

/**
 * @brief Initialize the framework with all memory still available to the OS heap.
 * @param argc Host argument count (unused).
 * @param ppArgv Host argument vector (unused).
 */
void ApplicationFunction::initialize(int argc, char** ppArgv) {
    sead::Framework::InitializeArg arg;
    arg.heap_size = gMaxMemoryBlockSize;
    sead::GameFrameworkNx::initialize(arg);
    al::setGpuPerformance(al::GpuPerformance_307MHz, nn::oe::PerformanceMode_Normal);
}
