#pragma once

#include <basis/seadTypes.h>

namespace al {
    class IUseNerve;
    class NerveStateBase;
    class Nerve;

    void setNerve(IUseNerve*, const Nerve*);
    void setNerveAtStep(IUseNerve*, const Nerve*, s32);
    bool isStep(const IUseNerve*, s32);
    bool isNerve(const IUseNerve*, const Nerve*);
    s32 getNerveStep(const IUseNerve*);
    const Nerve* getNerve(const IUseNerve*);
    bool isFirstStep(const IUseNerve*);
    bool isLessStep(const IUseNerve*, s32);
    bool isLessEqualStep(const IUseNerve*, s32);
    bool isGreaterStep(const IUseNerve*, s32);
    bool isGreaterEqualStep(const IUseNerve*, s32);
    bool isIntervalStep(const IUseNerve*, s32, s32);
    bool isIntervalOnOffStep(const IUseNerve*, s32, s32);
    bool isNewNerve(const IUseNerve*);
    f32 calcNerveRate(const IUseNerve*, s32);
    f32 calcNerveRate(const IUseNerve*, s32, s32);
    f32 calcNerveEaseInRate(const IUseNerve*, s32);
    f32 calcNerveEaseInRate(const IUseNerve*, s32, s32);
    f32 calcNerveEaseOutRate(const IUseNerve*, s32);
    f32 calcNerveEaseOutRate(const IUseNerve*, s32, s32);
    f32 calcNerveEaseInOutRate(const IUseNerve*, s32);
    f32 calcNerveEaseInOutRate(const IUseNerve*, s32, s32);
    f32 calcNerveSquareInRate(const IUseNerve*, s32);
    f32 calcNerveSquareInRate(const IUseNerve*, s32, s32);
    f32 calcNerveSquareOutRate(const IUseNerve*, s32);
    f32 calcNerveSquareOutRate(const IUseNerve*, s32, s32);
    f32 calcNerveValue(const IUseNerve*, s32, f32, f32);
    f32 calcNerveValue(const IUseNerve*, s32, s32, f32, f32);
    f32 calcNerveEaseInValue(const IUseNerve*, s32, f32, f32);
    f32 calcNerveEaseInValue(const IUseNerve*, s32, s32, f32, f32);
    f32 calcNerveEaseOutValue(const IUseNerve*, s32, f32, f32);
    f32 calcNerveEaseOutValue(const IUseNerve*, s32, s32, f32, f32);
    f32 calcNerveEaseInOutValue(const IUseNerve*, s32, f32, f32);
    f32 calcNerveEaseInOutValue(const IUseNerve*, s32, s32, f32, f32);
    f32 calcNerveSquareInValue(const IUseNerve*, s32, f32, f32);
    f32 calcNerveSquareInValue(const IUseNerve*, s32, s32, f32, f32);
    f32 calcNerveSquareOutValue(const IUseNerve*, s32, f32, f32);
    f32 calcNerveSquareOutValue(const IUseNerve*, s32, s32, f32, f32);
    f32 calcNerveJumpValue(const IUseNerve*, s32, s32, s32, f32);
    void initNerveState(IUseNerve*, NerveStateBase*, const Nerve*, const char*);
    void addNerveState(IUseNerve*, NerveStateBase*, const Nerve*, const char*);
    bool updateNerveState(IUseNerve*);
    bool updateNerveStateAndNextNerve(IUseNerve*, const Nerve*);
    bool isStateEnd(const IUseNerve*);
}  // namespace al

namespace alNerveFunction {
    void setNerveAction(al::IUseNerve*, const char*);
}  // namespace alNerveFunction
