#pragma once
#include <cstring>
#include <nn/atk/atk_Task.h>
#include <nn/atk/atk_PlayerHeapDataManager.h>
#include <nn/atk/atk_PlayerHeap.h>
#include <nn/atk/atk_SoundPlayer.h>

namespace nn::atk::detail::driver {
class WaveSoundLoader {
  public:
    struct Arg {
        /**
         * @brief Copies load parameters through the final wave-sound index, excluding trailing padding.
         * @param rArg Archive, player, and wave-sound selection to copy.
         */
        void Set(const Arg& rArg) {
            std::memcpy(this, &rArg, offsetof(Arg, waveSoundIndex) + sizeof(waveSoundIndex));
        }
        const SoundArchive* pArchive;
        const SoundDataManager* pDataManager;
        SoundPlayer* pSoundPlayer;
        u32 soundId;
        const void* pWaveSoundFile;
        u32 waveSoundIndex;
    };
    class DataLoadTask : public Task {
      public:
        /** @brief Releases the task's completion event through its base class. */
        ~DataLoadTask() override = default;
        void Initialize();
        bool TryAllocPlayerHeap();
        void Execute(TaskProfileLogger& rLogger) override;
        Arg arg;
        struct LoadResult {
            const void* pWaveSoundFile;
            const void* pWaveFile;
        };
        LoadResult result;
        PlayerHeap* pHeap;
        PlayerHeapDataManager* pHeapDataManager;
        bool succeeded;
    };
    class FreePlayerHeapTask : public Task {
      public:
        /** @brief Releases the task's completion event through its base class. */
        ~FreePlayerHeapTask() override = default;
        void Initialize();
        void Execute(TaskProfileLogger& rLogger) override;
        Arg arg;
        PlayerHeap* pHeap;
        PlayerHeapDataManager* pHeapDataManager;
    };
    ~WaveSoundLoader();
    void Initialize(const Arg& rArg);
    void Finalize();
    bool TryWait();
    bool IsInUse();

  private:
    /** @brief Waits until both load and heap-release tasks finish using this loader. */
    void WaitTasks() {
        os::WaitEvent(&mLoadTask.mCompletionEvent);
        os::WaitEvent(&mFreeTask.mCompletionEvent);
    }
    DataLoadTask mLoadTask;
    FreePlayerHeapTask mFreeTask;
    PlayerHeapDataManager mHeapDataManager;
};
static_assert(sizeof(WaveSoundLoader::Arg) == 0x30, "WaveSoundLoader argument size");
static_assert(sizeof(WaveSoundLoader::DataLoadTask) == 0xa0, "Wave data load task size");
static_assert(sizeof(WaveSoundLoader::FreePlayerHeapTask) == 0x88, "Wave heap release task size");
static_assert(sizeof(WaveSoundLoader) == 0x3f0, "WaveSoundLoader size");
} // namespace nn::atk::detail::driver
