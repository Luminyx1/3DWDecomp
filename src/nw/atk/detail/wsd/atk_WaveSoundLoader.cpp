#include <nn/atk/atk_WaveSoundLoader.h>
#include <nn/atk/atk_TaskManager.h>
#include <nn/atk/atk_SoundSystem.h>
#include <nn/atk/atk_Util.h>

namespace nn::atk::detail::driver {
namespace {
struct SoundIdWarningInfo : IWarningCallbackInfo {
    /** @brief Identifies the sound whose load failed. @param id Archive sound identifier. */
    explicit SoundIdWarningInfo(u32 id) : soundId(id) {}
    u32 soundId;
};
} // namespace

/** @brief Waits for outstanding tasks before destroying the embedded loader resources. */
WaveSoundLoader::~WaveSoundLoader() { WaitTasks(); }

/**
 * @brief Resets both tasks and installs the archive and player context for a new load.
 * @param rArg Load context; referenced archives and player must outlive the tasks.
 */
void WaveSoundLoader::Initialize(const Arg& rArg) {
    WaitTasks();
    mLoadTask.Initialize();
    mLoadTask.arg.Set(rArg);
    mLoadTask.pHeap = nullptr;
    mLoadTask.pHeapDataManager = &mHeapDataManager;
    mFreeTask.Initialize();
    mFreeTask.arg.Set(rArg);
    mFreeTask.pHeap = nullptr;
    mFreeTask.pHeapDataManager = &mHeapDataManager;
}

/** @brief Resets the loading task to idle with no published wave data. */
void WaveSoundLoader::DataLoadTask::Initialize() {
    mState = 0;
    std::memset(&result, 0, sizeof(result));
    succeeded = false;
}

/** @brief Resets the player-heap release task to idle. */
void WaveSoundLoader::FreePlayerHeapTask::Initialize() { mState = 0; }

/** @brief Schedules release of the player heap used by the last load. */
void WaveSoundLoader::Finalize() {
    mFreeTask.pHeap = mLoadTask.pHeap;
    TaskManager::GetInstance().AppendTask(&mFreeTask, TaskManager::TaskPriority_Normal);
}

/** @brief Starts an idle load when a player heap is available. @return True once loading completes or is
 * cancelled. */
bool WaveSoundLoader::TryWait() {
    if (!mLoadTask.TryAllocPlayerHeap()) {
        return false;
    }
    if (mLoadTask.mState == 3 || mLoadTask.mState == 4) {
        return true;
    }
    if (mLoadTask.mState == 0) {
        TaskManager::GetInstance().AppendTask(&mLoadTask, TaskManager::TaskPriority_Normal);
    }
    return false;
}

/** @brief Acquires a player heap if none is assigned. @return True if a heap is available for this task. */
bool WaveSoundLoader::DataLoadTask::TryAllocPlayerHeap() {
    if (pHeap == nullptr) {
        pHeap = arg.pSoundPlayer->detail_AllocPlayerHeap();
        if (pHeap == nullptr) {
            return false;
        }
    }
    return true;
}

/** @brief Checks whether either embedded task is still running. @return True while either completion event is
 * unsignalled. */
bool WaveSoundLoader::IsInUse() {
    return !os::TryWaitEvent(&mLoadTask.mCompletionEvent) || !os::TryWaitEvent(&mFreeTask.mCompletionEvent);
}

/**
 * @brief Loads wave-sound metadata and any missing wave archive into the player heap.
 * @param rLogger Unused task profiling destination in this implementation.
 */
void WaveSoundLoader::DataLoadTask::Execute(TaskProfileLogger& rLogger) {
    pHeapDataManager->Initialize(arg.pArchive);
    if (arg.pWaveSoundFile == nullptr && arg.soundId != SoundArchive::InvalidId) {
        u32 id = arg.soundId;
        if (!pHeapDataManager->LoadData(id, pHeap, 2, 0)) {
            SoundIdWarningInfo info(id);
            SoundSystem::CallWarningCallback(static_cast<WarningId>(1001), &info);
            pHeap->SetLoadFinished();
            succeeded = false;
            return;
        }
        arg.pWaveSoundFile = pHeapDataManager->detail_GetFileAddressByItemId(id);
    }
    const void* pWaveFile = Util::GetWaveFileOfWaveSound(arg.pWaveSoundFile, arg.waveSoundIndex,
                                                         *arg.pArchive, *arg.pDataManager);
    if (pWaveFile == nullptr) {
        if (!pHeapDataManager->detail_LoadWaveArchiveByWaveSoundFile(arg.pWaveSoundFile, arg.waveSoundIndex,
                                                                     pHeap)) {
            SoundIdWarningInfo info(arg.soundId);
            SoundSystem::CallWarningCallback(static_cast<WarningId>(1002), &info);
            pHeap->SetLoadFinished();
            succeeded = false;
            return;
        }
        pWaveFile = Util::GetWaveFileOfWaveSound(arg.pWaveSoundFile, arg.waveSoundIndex, *arg.pArchive,
                                                 *pHeapDataManager);
    }
    result.pWaveSoundFile = arg.pWaveSoundFile;
    result.pWaveFile = pWaveFile;
    pHeap->SetLoadFinished();
    succeeded = true;
}

/**
 * @brief Clears and returns the assigned player heap, then finalizes its data manager.
 * @param rLogger Unused task profiling destination in this implementation.
 */
void WaveSoundLoader::FreePlayerHeapTask::Execute(TaskProfileLogger& rLogger) {
    if (pHeap != nullptr) {
        pHeap->Clear();
        arg.pSoundPlayer->detail_FreePlayerHeap(pHeap);
    }
    pHeapDataManager->Finalize();
}
} // namespace nn::atk::detail::driver
