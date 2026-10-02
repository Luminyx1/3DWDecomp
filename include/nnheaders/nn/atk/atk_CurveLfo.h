#pragma once
#include <attributes.h>
#include <nn/types.h>

namespace nn::atk::detail {
using LfoCurveFunction = float (*)(float);
extern LfoCurveFunction g_CurveLfoTable[128] HIDDEN;
struct CurveLfoParam {
    void Initialize();
    float depth, speed;
    u32 delay;
    u8 range, curve, phase;
};
class CurveLfo {
public:
    static LfoCurveFunction RegisterUserCurve(LfoCurveFunction function, u32 index);
    static LfoCurveFunction UnregisterUserCurve(u32 index);
    static void InitializeCurveTable();
    void Reset();
    void Update(int step);
    float GetValue() const;

    CurveLfoParam mParameter;
    u32 mElapsedDelay;
    float mPhase;
    mutable float mRandomValue;
    bool mStarted, mWrapped;
};
static_assert(sizeof(CurveLfoParam) == 0x10, "CurveLfoParam size");
static_assert(sizeof(CurveLfo) == 0x20, "CurveLfo size");
}
