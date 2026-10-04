#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_BiquadFilterCallback.h>

namespace nn::atk {
/** @brief Destroy the filter callback interface without owning coefficient storage. */
BiquadFilterCallback::~BiquadFilterCallback() = default;
} // namespace nn::atk

namespace nn::atk::detail::driver {
/** @brief Lock the auxiliary effect list for this scope. */
HardwareManager::EffectAuxListScopedLock::EffectAuxListScopedLock() {
    HardwareManager::GetInstance().LockEffectAuxList();
}

/** @brief Release this scope's lock on the auxiliary effect list. */
HardwareManager::EffectAuxListScopedLock::~EffectAuxListScopedLock() {
    HardwareManager::GetInstance().UnlockEffectAuxList();
}

/** @brief Acquire exclusive access to the auxiliary effect list. */
void HardwareManager::LockEffectAuxList() { m_EffectAuxListMutex.Lock(); }

/** @brief Release exclusive access to the auxiliary effect list. */
void HardwareManager::UnlockEffectAuxList() { m_EffectAuxListMutex.Unlock(); }

/** @brief Lock the final-mix auxiliary effect list for this scope. */
HardwareManager::EffectAuxListForFinalMixScopedLock::EffectAuxListForFinalMixScopedLock() {
    HardwareManager::GetInstance().LockEffectAuxListForFinalMix();
}

/** @brief Release this scope's lock on the final-mix auxiliary effect list. */
HardwareManager::EffectAuxListForFinalMixScopedLock::~EffectAuxListForFinalMixScopedLock() {
    HardwareManager::GetInstance().UnlockEffectAuxListForFinalMix();
}

/** @brief Acquire exclusive access to the final-mix auxiliary effect list. */
void HardwareManager::LockEffectAuxListForFinalMix() { m_EffectAuxListForFinalMixMutex.Lock(); }

/** @brief Release exclusive access to the final-mix auxiliary effect list. */
void HardwareManager::UnlockEffectAuxListForFinalMix() { m_EffectAuxListForFinalMixMutex.Unlock(); }

/** @brief Lock the additional-submix auxiliary effect list for this scope. */
HardwareManager::EffectAuxListForAdditionalSubMixScopedLock::EffectAuxListForAdditionalSubMixScopedLock() {
    HardwareManager::GetInstance().LockEffectAuxListForAdditionalSubMix();
}

/** @brief Release this scope's lock on the additional-submix auxiliary effect list. */
HardwareManager::EffectAuxListForAdditionalSubMixScopedLock::~EffectAuxListForAdditionalSubMixScopedLock() {
    HardwareManager::GetInstance().UnlockEffectAuxListForAdditionalSubMix();
}

/** @brief Acquire exclusive access to the additional-submix auxiliary effect list. */
void HardwareManager::LockEffectAuxListForAdditionalSubMix() {
    m_EffectAuxListForAdditionalSubMixMutex.Lock();
}

/** @brief Release exclusive access to the additional-submix auxiliary effect list. */
void HardwareManager::UnlockEffectAuxListForAdditionalSubMix() {
    m_EffectAuxListForAdditionalSubMixMutex.Unlock();
}

/** @brief Lock the submix list for this scope. */
HardwareManager::SubMixListScopedLock::SubMixListScopedLock() {
    HardwareManager::GetInstance().LockSubMixList();
}

/** @brief Release this scope's lock on the submix list. */
HardwareManager::SubMixListScopedLock::~SubMixListScopedLock() {
    HardwareManager::GetInstance().UnlockSubMixList();
}

/** @brief Acquire exclusive access to the submix list. */
void HardwareManager::LockSubMixList() { m_SubMixListMutex.Lock(); }

/** @brief Release exclusive access to the submix list. */
void HardwareManager::UnlockSubMixList() { m_SubMixListMutex.Unlock(); }

/**
 * @brief Resolve built-in or custom submix topology and channel requirements.
 * @param enableStereoMode Limit the main mix to two channels instead of six.
 * @param enableEffect Reserve the extra mix channels required by effects.
 * @param enableSubMix Enable submix processing; required for either submix topology.
 * @param enableAdditionalEffectBus Add the optional effect bus; excludes custom submix topology.
 * @param enableAdditionalSubMix Add the optional submix; excludes custom submix topology.
 * @param enableCustomSubMix Request custom topology when no additional bus or submix is enabled.
 * @param customSubMixCount Requested custom submix count; used only when custom topology is accepted.
 * @param customChannelCount Custom channels in addition to the main mix; used only for custom topology.
 */
void HardwareManager::HardwareManagerParameter::SetSubMixParameter(
    bool enableStereoMode, bool enableEffect, bool enableSubMix, bool enableAdditionalEffectBus,
    bool enableAdditionalSubMix, bool enableCustomSubMix, int customSubMixCount, int customChannelCount) {
    bool custom = enableCustomSubMix && enableSubMix && !enableAdditionalEffectBus && !enableAdditionalSubMix;
    this->enableCustomSubMix = custom;
    this->enableSubMix = enableSubMix && !custom;
    this->enableAdditionalEffectBus = enableAdditionalEffectBus && !this->enableCustomSubMix;
    this->enableAdditionalSubMix = enableAdditionalSubMix && !this->enableCustomSubMix;
    int mainChannels = enableStereoMode ? 2 : 6;
    if (custom) {
        customChannelCount += mainChannels;
    } else {
        int channelCount = enableEffect ? 30 : 2 * mainChannels;
        int mixCount = 1;
        if (enableAdditionalEffectBus) {
            channelCount += enableEffect ? 6 : 2;
            ++mixCount;
        }
        if (enableAdditionalSubMix) {
            channelCount += enableEffect ? mainChannels : 0;
            ++mixCount;
        }
        customChannelCount = enableSubMix ? channelCount : mainChannels;
        customSubMixCount = enableSubMix ? mixCount : 0;
    }
    subMixCount = customSubMixCount;
    subMixTotalChannelCount = customChannelCount;
}
} // namespace nn::atk::detail::driver
