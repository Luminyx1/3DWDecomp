#pragma once

#include <basis/seadTypes.h>

namespace al {
class IScenarioCompleteChecker {
public:
    virtual bool isScenarioComplete(s32 zoneId, s32 scenarioId) = 0;
};
}  // namespace al
