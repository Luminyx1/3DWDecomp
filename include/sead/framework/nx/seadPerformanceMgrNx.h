#pragma once

#include <basis/seadTypes.h>
#include <nn/oe.h>

namespace sead
{
class PerformanceMgrNx
{
public:
    enum PowerMeasuringPoint
    {
    };

    static void initialize();
    static void setPerformanceConfiguration(nn::oe::PerformanceMode mode, s32 configuration);
    static void printPerformance();
    static void setCPUPerformance1122MHz();
    static bool isDockIn();
    static void startPowerMeasuring();
    static void setPeriodicPowerReportEnable(bool enable);
    static s32 getPowerMeasuringValue(PowerMeasuringPoint point);
    static s32 getAveragePowerMeasuringValue(PowerMeasuringPoint point);
    static void printAveragePowerMeasuringValue();
    static void resetAveragePowerMeasuring();
    static f32 toBatteryDurationHour(u16 value);
    static f32 getBatteryDurationHour();

private:
    static void printPerformanceForConfiguration_(s32 configuration);

    static s32 sPerformanceConfigurationForNormal;
    static s32 sPerformanceConfigurationForBoost;
};

}  // namespace sead
