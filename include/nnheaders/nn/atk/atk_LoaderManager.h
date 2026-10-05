#pragma once
#include <nn/atk/atk_SoundThread.h>

namespace nn::atk::detail {
/**
 * @brief Owns the storage and list roots for a sound runtime's loaders.
 * @tparam Loader Loader type managed by the runtime.
 */
template <typename Loader> class LoaderManager : public driver::SoundThread::SoundFrameCallback {
  public:
    /** @brief Constructs an empty loader manager. */
    LoaderManager() : mMemory(nullptr), mSize(0) {}
    /** @brief Destroys the manager after its owner releases the loaders. */
    ~LoaderManager() override = default;
    void OnBeginSoundFrame() override;

  private:
    void* mMemory;
    size_t mSize;
    util::IntrusiveListNode mFreeLoaders;
    util::IntrusiveListNode mActiveLoaders;
};
} // namespace nn::atk::detail
