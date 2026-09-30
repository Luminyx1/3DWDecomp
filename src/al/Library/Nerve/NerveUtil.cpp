#include "Library/Nerve/NerveUtil.hpp"

#include "Library/Nerve/IUseNerve.hpp"
#include "Library/Nerve/Nerve.hpp"
#include "Library/Nerve/NerveAction.hpp"
#include "Library/Nerve/NerveActionCtrl.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveStateBase.hpp"
#include "Library/Nerve/NerveStateCtrl.hpp"
#include "Library/Math/MathUtil.hpp"

namespace al {
    void setNerve(IUseNerve* pUser, const Nerve* pNerve) {
        pUser->getNerveKeeper()->setNerve(pNerve);
    }

    void setNerveAtStep(IUseNerve* pUser, const Nerve* pNerve, s32 step) {
        if (getNerveStep(pUser) == step)
            pUser->getNerveKeeper()->setNerve(pNerve);
    }

    bool isStep(const IUseNerve* pUser, s32 step) {
        return getNerveStep(pUser) == step;
    }

    bool isNerve(const IUseNerve* pUser, const Nerve* pNerve) {
        return getNerve(pUser) == pNerve;
    }

    s32 getNerveStep(const IUseNerve* pUser) {
        return pUser->getNerveKeeper()->mNerveStep;
    }

    const Nerve* getNerve(const IUseNerve* pUser) {
        return pUser->getNerveKeeper()->getCurrentNerve();
    }

    bool isFirstStep(const IUseNerve* pUser) {
        return getNerveStep(pUser) == 0;
    }

    bool isLessStep(const IUseNerve* pUser, s32 step) {
        return getNerveStep(pUser) < step;
    }

    bool isLessEqualStep(const IUseNerve* pUser, s32 step) {
        return getNerveStep(pUser) <= step;
    }

    bool isGreaterStep(const IUseNerve* pUser, s32 step) {
        return getNerveStep(pUser) > step;
    }

    bool isGreaterEqualStep(const IUseNerve* pUser, s32 step) {
        return getNerveStep(pUser) >= step;
    }

    bool isIntervalStep(const IUseNerve* pUser, s32 min, s32 max) {
        return (getNerveStep(pUser) - max) % min == 0;
    }

    bool isIntervalOnOffStep(const IUseNerve* pUser, s32 min, s32 max) {
        return (((getNerveStep(pUser) - max) / min) & 0x1) == 0;
    }

    bool isNewNerve(const IUseNerve* pUser) {
        return getNerveStep(pUser) >> 31;
    }

    /**
     * Calculates the nerve step divided by a duration, clamped to [0, 1].
     * @param pUser The nerve user.
     * @param max The duration.
     * @return The rate, or 1 if the duration is not positive.
     */
    f32 calcNerveRate(const IUseNerve* pUser, s32 max) {
        if (max < 1) {
            return 1.0f;
        }

        f32 rate = static_cast<f32>(getNerveStep(pUser)) / max;
        return sead::Mathf::clamp(rate, 0.0f, 1.0f);
    }

    /**
     * Calculates the nerve step normalized to a range, clamped to [0, 1].
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @return The rate.
     */
    f32 calcNerveRate(const IUseNerve* pUser, s32 min, s32 max) {
        f32 rate = normalize(static_cast<f32>(getNerveStep(pUser)), static_cast<f32>(min),
                             static_cast<f32>(max));
        return sead::Mathf::clamp(rate, 0.0f, 1.0f);
    }

    /**
     * Calculates the nerve rate over a duration with easeIn applied.
     * @param pUser The nerve user.
     * @param max The duration.
     * @return The eased rate.
     */
    f32 calcNerveEaseInRate(const IUseNerve* pUser, s32 max) {
        return easeIn(calcNerveRate(pUser, max));
    }

    /**
     * Calculates the nerve rate over a step range with easeIn applied.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @return The eased rate.
     */
    f32 calcNerveEaseInRate(const IUseNerve* pUser, s32 min, s32 max) {
        return easeIn(calcNerveRate(pUser, min, max));
    }

    /**
     * Calculates the nerve rate over a duration with easeOut applied.
     * @param pUser The nerve user.
     * @param max The duration.
     * @return The eased rate.
     */
    f32 calcNerveEaseOutRate(const IUseNerve* pUser, s32 max) {
        return easeOut(calcNerveRate(pUser, max));
    }

    /**
     * Calculates the nerve rate over a step range with easeOut applied.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @return The eased rate.
     */
    f32 calcNerveEaseOutRate(const IUseNerve* pUser, s32 min, s32 max) {
        return easeOut(calcNerveRate(pUser, min, max));
    }

    /**
     * Calculates the nerve rate over a duration with easeInOut applied.
     * @param pUser The nerve user.
     * @param max The duration.
     * @return The eased rate.
     */
    f32 calcNerveEaseInOutRate(const IUseNerve* pUser, s32 max) {
        return easeInOut(calcNerveRate(pUser, max));
    }

    /**
     * Calculates the nerve rate over a step range with easeInOut applied.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @return The eased rate.
     */
    f32 calcNerveEaseInOutRate(const IUseNerve* pUser, s32 min, s32 max) {
        return easeInOut(calcNerveRate(pUser, min, max));
    }

    /**
     * Calculates the nerve rate over a duration with squareIn applied.
     * @param pUser The nerve user.
     * @param max The duration.
     * @return The eased rate.
     */
    f32 calcNerveSquareInRate(const IUseNerve* pUser, s32 max) {
        return squareIn(calcNerveRate(pUser, max));
    }

    /**
     * Calculates the nerve rate over a step range with squareIn applied.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @return The eased rate.
     */
    f32 calcNerveSquareInRate(const IUseNerve* pUser, s32 min, s32 max) {
        return squareIn(calcNerveRate(pUser, min, max));
    }

    /**
     * Calculates the nerve rate over a duration with squareOut applied.
     * @param pUser The nerve user.
     * @param max The duration.
     * @return The eased rate.
     */
    f32 calcNerveSquareOutRate(const IUseNerve* pUser, s32 max) {
        return squareOut(calcNerveRate(pUser, max));
    }

    /**
     * Calculates the nerve rate over a step range with squareOut applied.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @return The eased rate.
     */
    f32 calcNerveSquareOutRate(const IUseNerve* pUser, s32 min, s32 max) {
        return squareOut(calcNerveRate(pUser, min, max));
    }

    /**
     * Interpolates between two values by the linear nerve rate over a duration.
     * @param pUser The nerve user.
     * @param max The duration.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveRate(pUser, max), start, end);
    }

    /**
     * Interpolates between two values by the linear nerve rate over a step range.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveRate(pUser, min, max), start, end);
    }

    /**
     * Interpolates between two values by the EaseIn nerve rate over a duration.
     * @param pUser The nerve user.
     * @param max The duration.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveEaseInValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveEaseInRate(pUser, max), start, end);
    }

    /**
     * Interpolates between two values by the EaseIn nerve rate over a step range.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveEaseInValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveEaseInRate(pUser, min, max), start, end);
    }

    /**
     * Interpolates between two values by the EaseOut nerve rate over a duration.
     * @param pUser The nerve user.
     * @param max The duration.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveEaseOutValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveEaseOutRate(pUser, max), start, end);
    }

    /**
     * Interpolates between two values by the EaseOut nerve rate over a step range.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveEaseOutValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveEaseOutRate(pUser, min, max), start, end);
    }

    /**
     * Interpolates between two values by the EaseInOut nerve rate over a duration.
     * @param pUser The nerve user.
     * @param max The duration.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveEaseInOutValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveEaseInOutRate(pUser, max), start, end);
    }

    /**
     * Interpolates between two values by the EaseInOut nerve rate over a step range.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveEaseInOutValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveEaseInOutRate(pUser, min, max), start, end);
    }

    /**
     * Interpolates between two values by the SquareIn nerve rate over a duration.
     * @param pUser The nerve user.
     * @param max The duration.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveSquareInValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveSquareInRate(pUser, max), start, end);
    }

    /**
     * Interpolates between two values by the SquareIn nerve rate over a step range.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveSquareInValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveSquareInRate(pUser, min, max), start, end);
    }

    /**
     * Interpolates between two values by the SquareOut nerve rate over a duration.
     * @param pUser The nerve user.
     * @param max The duration.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveSquareOutValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveSquareOutRate(pUser, max), start, end);
    }

    /**
     * Interpolates between two values by the SquareOut nerve rate over a step range.
     * @param pUser The nerve user.
     * @param min The start step.
     * @param max The end step.
     * @param start The value at the start.
     * @param end The value at the end.
     * @return The interpolated value.
     */
    f32 calcNerveSquareOutValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
        return lerpValue(calcNerveSquareOutRate(pUser, min, max), start, end);
    }

    /**
     * Calculates a jump curve that rises, holds and falls over the nerve steps.
     * @param pUser The nerve user.
     * @param riseSteps The duration of the rise.
     * @param holdSteps The duration of the hold.
     * @param fallSteps The duration of the fall.
     * @param height The jump height.
     * @return The current height.
     */
    f32 calcNerveJumpValue(const IUseNerve* pUser, s32 riseSteps, s32 holdSteps, s32 fallSteps,
                           f32 height) {
        s32 step = getNerveStep(pUser);
        if (step <= riseSteps) {
            return calcNerveEaseOutRate(pUser, riseSteps) * height;
        }

        s32 fallStart = holdSteps + riseSteps;
        if (step <= fallStart) {
            return height;
        }

        return lerpValue(calcNerveEaseInRate(pUser, fallStart, fallStart + fallSteps), height, 0.0f);
    }

    /**
     * Initializes a state and registers it for a nerve.
     * @param pUser The nerve user.
     * @param pState The state.
     * @param pNerve The nerve that runs the state.
     * @param pName The state name.
     */
    void initNerveState(IUseNerve* pUser, NerveStateBase* pState, const Nerve* pNerve,
                        const char* pName) {
        pState->init();
        addNerveState(pUser, pState, pNerve, pName);
    }

    /**
     * Registers a state for a nerve.
     * @param pUser The nerve user.
     * @param pState The state.
     * @param pNerve The nerve that runs the state.
     * @param pName The state name.
     */
    void addNerveState(IUseNerve* pUser, NerveStateBase* pState, const Nerve* pNerve,
                       const char* pName) {
        pUser->getNerveKeeper()->mStateCtrl->addState(pState, pNerve, pName);
    }

    /**
     * Updates the current state.
     * @param pUser The nerve user.
     * @return Whether the state ended.
     */
    bool updateNerveState(IUseNerve* pUser) {
        return pUser->getNerveKeeper()->mStateCtrl->updateCurrentState();
    }

    /**
     * Updates the current state and changes the nerve if it ended.
     * @param pUser The nerve user.
     * @param pNerve The nerve to change to.
     * @return Whether the state ended.
     */
    bool updateNerveStateAndNextNerve(IUseNerve* pUser, const Nerve* pNerve) {
        if (pUser->getNerveKeeper()->mStateCtrl->updateCurrentState()) {
            pUser->getNerveKeeper()->setNerve(pNerve);
            return true;
        }

        return false;
    }

    /**
     * Checks whether the current state ended.
     * @param pUser The nerve user.
     * @return Whether the state ended.
     */
    bool isStateEnd(const IUseNerve* pUser) {
        return pUser->getNerveKeeper()->mStateCtrl->isCurrentStateEnd();
    }
};

namespace alNerveFunction {
    /**
     * Changes the nerve to the nerve action with a name.
     * @param pUser The nerve user.
     * @param pName The action name.
     */
    void setNerveAction(al::IUseNerve* pUser, const char* pName) {
        al::NerveKeeper* keeper = pUser->getNerveKeeper();
        keeper->setNerve(keeper->mActionCtrl->findNerve(pName));
    }
}  // namespace alNerveFunction
