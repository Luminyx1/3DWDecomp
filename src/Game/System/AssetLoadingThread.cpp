#include "System/AssetLoadingThread.hpp"
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <nn/os.h>
#include <prim/seadDelegate.h>
#include <prim/seadEnum.h>
#include <thread/seadDelegateThread.h>
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/File/FileUtil.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/System/GameSystemInfo.hpp"
#include "Project/Audio/System/AudioSystem.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {

// clang-format off
SEAD_ENUM(StationedCategory, MainTitle, MainTitle2nd, PreTitle3DWorld, InTitle3DWorld, CourseSelect3DWorld, SingleMode, SingleModeStageData, INVALID)
// clang-format on

typedef sead::Delegate2<rc::AssetLoadingThread, sead::Thread*, s64> ThreadDelegate;

const s32 cCategoryNumPerLoadType = 4;

/// Thread priorities used by the asset-loading thread.
struct ThreadPriority {
    s32 load;
    s32 normal;
};

ThreadPriority sThreadPriority = {sead::Thread::cDefaultPriority + 5,
                                  sead::Thread::cDefaultPriority};

const char* const cLoadHeapNames[rc::AssetLoadingThread::LOAD_TYPE_NUM] = {
    "Stationed[Shared]",
    "Stationed[Pre3DWorldTitle]",
    "Stationed[In3DWorldTitle]",
    "Stationed[SingleMode]",
};

const StationedCategory cLoadCategories[rc::AssetLoadingThread::LOAD_TYPE_NUM]
                                       [cCategoryNumPerLoadType] = {
    {StationedCategory::MainTitle, StationedCategory::MainTitle2nd, StationedCategory::INVALID,
     StationedCategory::INVALID},
    {StationedCategory::PreTitle3DWorld, StationedCategory::INVALID, StationedCategory::INVALID,
     StationedCategory::INVALID},
    {StationedCategory::InTitle3DWorld, StationedCategory::CourseSelect3DWorld,
     StationedCategory::INVALID, StationedCategory::INVALID},
    {StationedCategory::SingleMode, StationedCategory::SingleModeStageData,
     StationedCategory::INVALID, StationedCategory::INVALID},
};

} // namespace

rc::AssetLoadingThread* rc::AssetLoadingThread::sInstance = nullptr;

/**
 * @brief Create the asset-loading thread singleton if it does not exist yet.
 * @param pHeap Parent heap for the per-load-type stationed heaps.
 * @return Always nullptr.
 */
rc::AssetLoadingThread* rc::AssetLoadingThread::createInstance(sead::Heap* pHeap) {
    if (sInstance == nullptr) {
        sInstance = new AssetLoadingThread(pHeap);
    }

    return nullptr;
}

/**
 * @brief Destroy the asset-loading thread singleton.
 */
void rc::AssetLoadingThread::deleteInstance() {
    if (sInstance != nullptr) {
        delete sInstance;
        sInstance = nullptr;
    }
}

/**
 * @brief Create the loading events and heap table, then start the loading thread.
 * @param pHeap Parent heap for the per-load-type stationed heaps.
 */
rc::AssetLoadingThread::AssetLoadingThread(sead::Heap* pHeap)
    : mpThread(nullptr), mpParentHeap(pHeap), mpLoadHeaps(nullptr), mFastLoad(true) {
    mpCancelEvent = new sead::Event(true);
    mpThreadIdleEvent = new sead::Event(true);
    mpDoneEvents = new sead::Event[LOAD_TYPE_NUM];
    mpLoadingEvents = new sead::Event[LOAD_TYPE_NUM];
    mpLoadHeaps = new sead::Heap*[LOAD_TYPE_NUM];

    for (s32 i = 0; i < LOAD_TYPE_NUM; i++) {
        mpDoneEvents[i].initialize(true);
        mpLoadingEvents[i].initialize(true);
        mpLoadHeaps[i] = nullptr;
    }

    mpThread = new sead::DelegateThread(
        "AssetLoadingThread", new ThreadDelegate(this, &AssetLoadingThread::threadMsgReceive),
        nullptr, sThreadPriority.load, sead::MessageQueue::BlockType::Blocking, 0x7fffffff,
        0x20000, 0x10);
    mpThread->start();
}

/**
 * @brief Load every stationed resource category of one load type.
 * @param pThread The loading thread (unused).
 * @param msg Load type plus one, as sent by startLoad().
 */
void rc::AssetLoadingThread::threadMsgReceive(sead::Thread* pThread, s64 msg) {
    s64 type = msg - 1;
    mpThreadIdleEvent->resetSignal();

    if (mFastLoad) {
        al::setCpuBoost(true, false);
    }

    if (type == LOAD_TYPE_SHARED) {
        al::clearFileLoaderEntry();
        alAudioSystemFunction::loadResourceFromUserManagementFile(
            al::UMF_SE_STATIONED_1ST, mpAudioSystemInfo->getSeadAudioPlayerForSe(), false);
        alAudioSystemFunction::loadResourceFromUserManagementFile(
            al::UMF_SE_STATIONED_2ND, mpAudioSystemInfo->getSeadAudioPlayerForSe(), false);
        alAudioSystemFunction::loadResourceFromUserManagementFile(
            al::UMF_BGM_STATIONED_2ND, mpAudioSystemInfo->getSeadAudioPlayerForBgm(), false);
        al::clearFileLoaderEntry();
    }

    const char* heapName = cLoadHeapNames[type];
    mpLoadHeaps[type] = sead::ExpHeap::create(0, heapName, mpParentHeap, 4,
                                              sead::Heap::cHeapDirection_Forward, false);
    sead::Heap* pHeap = mpLoadHeaps[type];
    al::addNamedHeap(pHeap, heapName);

    for (s32 i = 0; i < cCategoryNumPerLoadType; i++) {
        StationedCategory category = cLoadCategories[type][i];
        if (category == StationedCategory::INVALID) {
            break;
        }

        al::StringTmp<64> categoryName("Stationed[%s]", category.text());
        sead::ScopedCurrentHeapSetter heapSetter(pHeap);
        al::addResourceCategory(categoryName.cstr(), 0x400, pHeap);
        al::clearFileLoaderEntry();
        if (!al::createCategoryResourceAll(categoryName.cstr(), mpCancelEvent)) {
            al::clearFileLoaderEntry();
            pHeap->adjust();
            al::setCpuBoost(false, false);
            mpLoadingEvents[type].resetSignal();
            mpThreadIdleEvent->setSignal();
            return;
        }
    }

    al::clearFileLoaderEntry();
    pHeap->adjust();
    al::setCpuBoost(false, false);
    mpLoadingEvents[type].resetSignal();
    mpDoneEvents[type].setSignal();
    mpThreadIdleEvent->setSignal();
}

/**
 * @brief Destroy the asset-loading thread object.
 */
rc::AssetLoadingThread::~AssetLoadingThread() {}

/**
 * @brief Request a background load unless it is already loading or loaded.
 * @param type Load category index from 0 through 3.
 */
void rc::AssetLoadingThread::startLoad(LOAD_TYPE type) {
    if (!mpDoneEvents[type].wait(sead::TickSpan(0)) &&
        !mpLoadingEvents[type].wait(sead::TickSpan(0))) {
        mpLoadingEvents[type].setSignal();
        mpThread->sendMessage(static_cast<s32>(type + 1),
                              sead::MessageQueue::BlockType::NonBlocking);
    }
}

/**
 * @brief Block until a load in progress has finished.
 * @param type Load category index from 0 through 3.
 */
void rc::AssetLoadingThread::waitLoadDone(LOAD_TYPE type) {
    if (!isLoading(type)) {
        return;
    }

    while (!mpDoneEvents[type].wait(sead::TickSpan::makeFromMilliSeconds(6))) {
    }
}

/**
 * @brief Poll an asset-loading event without blocking.
 * @param type Load category index from 0 through 3.
 * @return True when the corresponding event is signaled.
 */
bool rc::AssetLoadingThread::isLoading(LOAD_TYPE type) {
    return mpLoadingEvents[type].wait(sead::TickSpan(0));
}

/**
 * @brief Sleep until no load type is loading anymore.
 */
void rc::AssetLoadingThread::waitAllLoadingDone() {
    while (!isAllLoadingDone()) {
        nn::os::SleepThread(nn::TimeSpan::FromMilliSeconds(5));
    }
}

/**
 * @brief Check whether no load type is currently loading.
 * @return True when every load type is idle.
 */
bool rc::AssetLoadingThread::isAllLoadingDone() {
    for (u32 i = 0; i < LOAD_TYPE_NUM; i++) {
        if (isLoading(static_cast<LOAD_TYPE>(i))) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Poll an asset-loading event without blocking.
 * @param type Load category index from 0 through 3.
 * @return True when the corresponding event is signaled.
 */
bool rc::AssetLoadingThread::isLoadDone(LOAD_TYPE type) {
    return mpDoneEvents[type].wait(sead::TickSpan(0));
}

/**
 * @brief Check whether assets are loading or already loaded.
 * @param type Load category index from 0 through 3.
 * @return True when either loading or completion is signaled.
 */
bool rc::AssetLoadingThread::isLoadOrLoading(LOAD_TYPE type) {
    return isLoading(type) || isLoadDone(type);
}

/**
 * @brief Toggle fast loading and release the CPU boost when disabled.
 * @param disable True to disable fast loading; false to enable it.
 */
void rc::AssetLoadingThread::disableFastLoad(bool disable) {
    if (mFastLoad == disable) {
        mFastLoad = !disable;
        if (!mFastLoad) {
            al::setCpuBoost(false, false);
        }
    }
}

/**
 * @brief Cancel and unload both 3D World stationed load types.
 */
void rc::AssetLoadingThread::cancelAndDelete3DWorldStationed() {
    cancelAndDeleteLoad(LOAD_TYPE_IN_3D_WORLD_TITLE);
    cancelAndDeleteLoad(LOAD_TYPE_PRE_3D_WORLD_TITLE);
}

/**
 * @brief Cancel a running load and remove its resource categories and heap.
 * @param type Load category index from 0 through 3.
 */
void rc::AssetLoadingThread::cancelAndDeleteLoad(LOAD_TYPE type) {
    while (isLoading(type)) {
        mpCancelEvent->setSignal();
        while (!mpThreadIdleEvent->wait(sead::TickSpan::makeFromMilliSeconds(6))) {
        }
    }

    mpCancelEvent->resetSignal();
    if (mpLoadHeaps[type] == nullptr) {
        return;
    }

    for (s32 i = 0; i < cCategoryNumPerLoadType; i++) {
        StationedCategory category = cLoadCategories[type][i];
        if (category == StationedCategory::INVALID) {
            break;
        }

        al::StringTmp<64> categoryName("Stationed[%s]", category.text());
        if (al::isCategoryAdded(categoryName.cstr())) {
            al::removeResourceCategory(categoryName.cstr());
        }
    }

    al::removeNamedHeap(cLoadHeapNames[type]);
    mpLoadHeaps[type]->freeAll();
    mpLoadHeaps[type]->destroy();
    mpLoadHeaps[type] = nullptr;
    mpDoneEvents[type].resetSignal();
}

/**
 * @brief Cancel and unload the single-mode stationed load type.
 */
void rc::AssetLoadingThread::cancelAndDeleteSingleModeStationed() {
    cancelAndDeleteLoad(LOAD_TYPE_SINGLE_MODE);
}
