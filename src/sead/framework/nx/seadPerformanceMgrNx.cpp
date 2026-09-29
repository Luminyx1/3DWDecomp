#include <framework/nx/seadPerformanceMgrNx.h>

#include <math.h>

namespace sead
{
s32 PerformanceMgrNx::sPerformanceConfigurationForNormal = 0x20003;
s32 PerformanceMgrNx::sPerformanceConfigurationForBoost = 0x10001;

/**
 * Initializes the operating environment library.
 */
void PerformanceMgrNx::initialize()
{
    nn::oe::Initialize();
}

/**
 * Sets and remembers the performance configuration of a performance mode.
 * @param mode the performance mode
 * @param configuration the configuration to use in that mode
 */
void PerformanceMgrNx::setPerformanceConfiguration(nn::oe::PerformanceMode mode, s32 configuration)
{
    s32* stored = mode == nn::oe::PerformanceMode_Boost ? &sPerformanceConfigurationForBoost :
                                                          &sPerformanceConfigurationForNormal;
    *stored = configuration;
    nn::oe::SetPerformanceConfiguration(mode, configuration);
}

/**
 * Prints the current performance settings; does nothing in release builds.
 */
void PerformanceMgrNx::printPerformance() {}

/**
 * Prints a performance configuration; does nothing in release builds.
 * @param configuration the configuration to print
 */
void PerformanceMgrNx::printPerformanceForConfiguration_(s32 configuration) {}

/**
 * Switches the CPU to 1122 MHz; unsupported in release builds.
 */
void PerformanceMgrNx::setCPUPerformance1122MHz() {}

/**
 * Checks whether the console is docked; unsupported in release builds.
 * @return always false
 */
bool PerformanceMgrNx::isDockIn()
{
    return false;
}

/**
 * Starts measuring power consumption; unsupported in release builds.
 */
void PerformanceMgrNx::startPowerMeasuring() {}

/**
 * Enables periodic power reports; unsupported in release builds.
 * @param enable whether reports are enabled
 */
void PerformanceMgrNx::setPeriodicPowerReportEnable(bool enable) {}

/**
 * Gets a measured power value; unsupported in release builds.
 * @param point the measuring point
 * @return always 0
 */
s32 PerformanceMgrNx::getPowerMeasuringValue(PowerMeasuringPoint point)
{
    return 0;
}

/**
 * Gets an averaged measured power value; unsupported in release builds.
 * @param point the measuring point
 * @return always 0
 */
s32 PerformanceMgrNx::getAveragePowerMeasuringValue(PowerMeasuringPoint point)
{
    return 0;
}

/**
 * Prints the averaged power values; does nothing in release builds.
 */
void PerformanceMgrNx::printAveragePowerMeasuringValue() {}

/**
 * Resets the averaged power values; does nothing in release builds.
 */
void PerformanceMgrNx::resetAveragePowerMeasuring() {}

/**
 * Converts a battery gauge reading into an estimated remaining duration in hours.
 * @param value the battery gauge reading
 * @return the estimated duration in hours
 */
f32 PerformanceMgrNx::toBatteryDurationHour(u16 value)
{
    if (value < 4983)
    {
        value = 4983;
    }

    f32 t = logf(value) * 7769.6f + -62356.0f;
    f32 capacity =
        (t * (t * 8.5f / 1000.0f) / 1000.0f + t * -217.5f / 1000.0f + 5173.0f) * 3.6f / 1000.0f;
    f32 ratio = t * (t * -0.0052f / 1000.0f) / 1000.0f + 1.0f + t * 0.0718f / 1000.0f + -0.1238f;
    return ratio * capacity / t * 1000.0f;
}

/**
 * Gets the estimated remaining battery duration; unsupported in release builds.
 * @return always 0
 */
f32 PerformanceMgrNx::getBatteryDurationHour()
{
    return 0.0f;
}

}  // namespace sead
