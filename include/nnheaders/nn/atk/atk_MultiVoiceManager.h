#pragma once

#include <nn/types.h>

namespace nn::atk::detail::driver {
/** @brief Driver-side owner of every active multi-channel voice. */
class MultiVoiceManager {
  public:
    static MultiVoiceManager& GetInstance();
    /**
     * @brief Synchronously refresh parameters of all voices.
     * @param updateFlag Bit set selecting which parameters to recompute.
     */
    void UpdateAllVoicesSync(u32 updateFlag);
};
} // namespace nn::atk::detail::driver
