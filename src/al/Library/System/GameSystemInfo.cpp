#include "Library/System/GameSystemInfo.hpp"

#include "Library/Application/ApplicationMessageReceiver.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"

namespace al {
namespace {
bool sIsCpuBoostOn = false;
s32 sCpuBoostOffDelay = 0;

inline nn::oe::PerformanceMode getPerformanceMode(const ApplicationMessageReceiver* pReceiver) {
    switch (pReceiver->getCachedPerformanceMode()) {
    case nn::oe::PerformanceMode_Normal:
        return nn::oe::PerformanceMode_Normal;
    case nn::oe::PerformanceMode_Boost:
        return nn::oe::PerformanceMode_Boost;
    default:
        return nn::oe::PerformanceMode_Invalid;
    }
}

inline const ApplicationMessageReceiver* getApplicationMessageReceiver(const LiveActor* pActor) {
    const GraphicsSystemInfo* info = static_cast<GraphicsSystemInfo*>(pActor->getSceneInfo()->_78);
    return *reinterpret_cast<ApplicationMessageReceiver* const*>(&info->_d70[0x1040 - 0xd70]);
}

void setPerformanceConfiguration(nn::oe::PerformanceMode mode, u64 cpu, u64 gpu, u64 memory) {
    switch (mode) {
    case nn::oe::PerformanceMode_Normal:
        switch (gpu) {
        case GpuPerformance_768MHz:
            if (memory == MemoryPerformance_1331MHz) {
                nn::oe::SetPerformanceConfiguration(mode, 0x20003);
            }

            break;
        case GpuPerformance_307MHz:
            if (memory == MemoryPerformance_1331MHz) {
                nn::oe::SetPerformanceConfiguration(mode, 0x20004);
            }

            break;
        default:
            break;
        }

        break;
    case nn::oe::PerformanceMode_Boost:
        switch (gpu) {
        case GpuPerformance_768MHz:
            if (memory == MemoryPerformance_1331MHz) {
                nn::oe::SetPerformanceConfiguration(mode, 0x20003);
            }

            break;
        case GpuPerformance_307MHz:
            if (memory == MemoryPerformance_1331MHz) {
                nn::oe::SetPerformanceConfiguration(mode, 0x20004);
            }

            break;
        case GpuPerformance_Boost:
            if (memory == MemoryPerformance_1600MHz) {
                nn::oe::SetPerformanceConfiguration(mode, 0x10001);
            }

            break;
        default:
            break;
        }

        break;
    default:
        break;
    }
}
}  // namespace

/**
 * Checks if the console runs in normal performance mode.
 * @param pReceiver application message receiver
 * @return whether the performance mode is normal
 */
bool isPerformanceNormal(const ApplicationMessageReceiver* pReceiver) {
    return pReceiver->getCachedPerformanceMode() == nn::oe::PerformanceMode_Normal;
}

/**
 * Checks if the console runs in boost performance mode.
 * @param pReceiver application message receiver
 * @return whether the performance mode is boost
 */
bool isPerformanceBoost(const ApplicationMessageReceiver* pReceiver) {
    return pReceiver->getCachedPerformanceMode() == nn::oe::PerformanceMode_Boost;
}

/**
 * Gets the performance configuration of a performance mode.
 * @param mode performance mode
 * @return performance configuration
 */
nn::oe::PerformanceConfiguration getPerformanceConfiguration(nn::oe::PerformanceMode mode) {
    return nn::oe::GetPerformanceConfiguration(mode);
}

/**
 * Gets the performance configuration of the current performance mode.
 * @param pReceiver application message receiver
 * @return performance configuration
 */
nn::oe::PerformanceConfiguration
getPerformanceConfiguration(const ApplicationMessageReceiver* pReceiver) {
    return getPerformanceConfiguration(getPerformanceMode(pReceiver));
}

/**
 * Gets the CPU performance of a performance mode.
 * @param mode performance mode
 * @return CPU performance
 */
u64 getCpuPerformance(nn::oe::PerformanceMode mode) {
    switch (getPerformanceConfiguration(mode)) {
    case 0x10001:
    case 0x20003:
    case 0x20004:
        return CpuPerformance_1020MHz;
    default:
        return CpuPerformance_Invalid;
    }
}

/**
 * Gets the CPU performance of the current performance mode.
 * @param pReceiver application message receiver
 * @return CPU performance
 */
u64 getCpuPerformance(const ApplicationMessageReceiver* pReceiver) {
    return getCpuPerformance(getPerformanceMode(pReceiver));
}

/**
 * Gets the CPU performance of the current performance mode.
 * @param pActor actor
 * @return CPU performance
 */
u64 getCpuPerformance(const LiveActor* pActor) {
    return getCpuPerformance(getApplicationMessageReceiver(pActor));
}

/**
 * Gets the GPU performance of a performance mode.
 * @param mode performance mode
 * @return GPU performance
 */
u64 getGpuPerformance(nn::oe::PerformanceMode mode) {
    switch (getPerformanceConfiguration(mode)) {
    case 0x10001:
        return GpuPerformance_Boost;
    case 0x20004:
        return GpuPerformance_307MHz;
    case 0x20003:
        return GpuPerformance_768MHz;
    default:
        return GpuPerformance_Invalid;
    }
}

/**
 * Gets the GPU performance of the current performance mode.
 * @param pReceiver application message receiver
 * @return GPU performance
 */
u64 getGpuPerformance(const ApplicationMessageReceiver* pReceiver) {
    return getGpuPerformance(getPerformanceMode(pReceiver));
}

/**
 * Gets the GPU performance of the current performance mode.
 * @param pActor actor
 * @return GPU performance
 */
u64 getGpuPerformance(const LiveActor* pActor) {
    return getGpuPerformance(getApplicationMessageReceiver(pActor));
}

/**
 * Gets the memory performance of a performance mode.
 * @param mode performance mode
 * @return memory performance
 */
u64 getMemoryPerformance(nn::oe::PerformanceMode mode) {
    nn::oe::PerformanceConfiguration config = getPerformanceConfiguration(mode);

    if (config == 0x20003 || config == 0x20004) {
        return MemoryPerformance_1331MHz;
    }

    return config == 0x10001 ? MemoryPerformance_1600MHz : MemoryPerformance_Invalid;
}

/**
 * Gets the memory performance of the current performance mode.
 * @param pReceiver application message receiver
 * @return memory performance
 */
u64 getMemoryPerformance(const ApplicationMessageReceiver* pReceiver) {
    return getMemoryPerformance(getPerformanceMode(pReceiver));
}

/**
 * Gets the memory performance of the current performance mode.
 * @param pActor actor
 * @return memory performance
 */
u64 getMemoryPerformance(const LiveActor* pActor) {
    return getMemoryPerformance(getApplicationMessageReceiver(pActor));
}

/**
 * Does nothing.
 * @param performance CPU performance
 * @param mode performance mode
 */
void setCpuPerformance(CpuPerformance performance, nn::oe::PerformanceMode mode) {}

/**
 * Does nothing.
 * @param performance CPU performance
 * @param pReceiver application message receiver
 */
void setCpuPerformance(CpuPerformance performance, const ApplicationMessageReceiver* pReceiver) {}

/**
 * Sets the GPU performance of a performance mode.
 * @param performance GPU performance
 * @param mode performance mode
 */
void setGpuPerformance(GpuPerformance performance, nn::oe::PerformanceMode mode) {
    setPerformanceConfiguration(mode, getCpuPerformance(mode), performance,
                                getMemoryPerformance(mode));
}

/**
 * Sets the GPU performance of the current performance mode.
 * @param performance GPU performance
 * @param pReceiver application message receiver
 */
void setGpuPerformance(GpuPerformance performance, const ApplicationMessageReceiver* pReceiver) {
    setGpuPerformance(performance, getPerformanceMode(pReceiver));
}

/**
 * Sets the memory performance of a performance mode.
 * @param performance memory performance
 * @param mode performance mode
 */
void setMemoryPerformance(MemoryPerformance performance, nn::oe::PerformanceMode mode) {
    setPerformanceConfiguration(mode, getCpuPerformance(mode), getGpuPerformance(mode),
                                performance);
}

/**
 * Sets the memory performance of the current performance mode.
 * @param performance memory performance
 * @param pReceiver application message receiver
 */
void setMemoryPerformance(MemoryPerformance performance,
                          const ApplicationMessageReceiver* pReceiver) {
    setMemoryPerformance(performance, getPerformanceMode(pReceiver));
}

/**
 * Checks if the CPU boost is on.
 * @return whether the CPU boost is on
 */
bool isCpuBoostOn() {
    return sIsCpuBoostOn;
}

/**
 * Turns the CPU boost on, or schedules turning it off.
 * @param isBoost whether the CPU boost is requested
 * @param isUnused unused
 */
void setCpuBoost(bool isBoost, bool isUnused) {
    sCpuBoostOffDelay = isBoost ? 0 : 4;

    if (sIsCpuBoostOn == isBoost) {
        return;
    }

    if (isBoost) {
        nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode_Enabled);
        sIsCpuBoostOn = isBoost;
    }
}

/**
 * Turns the CPU boost off once its off delay ran out.
 */
void updateCpuBoost() {
    if (sCpuBoostOffDelay > 0) {
        sCpuBoostOffDelay--;

        if (sCpuBoostOffDelay == 0) {
            nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode_Disabled);
            sIsCpuBoostOn = false;
        }
    }
}

/**
 * Constructs an empty game system info.
 */
GameSystemInfo::GameSystemInfo() = default;
}  // namespace al
