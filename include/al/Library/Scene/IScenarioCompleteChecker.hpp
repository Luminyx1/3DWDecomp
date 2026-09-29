#pragma once

#include <basis/seadTypes.h>

namespace al {
    /// Checks whether a scenario of a zone has been completed.
    class IScenarioCompleteChecker {
    public:
        virtual bool isCompleteScenario(s32 zoneId, s32 scenarioId) const = 0;
    };
};
