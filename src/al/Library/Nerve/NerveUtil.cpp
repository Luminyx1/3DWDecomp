#include "Library/Nerve/NerveUtil.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/IUseNerve.hpp"
#include "Library/Nerve/Nerve.hpp"
#include "Library/Nerve/NerveAction.hpp"
#include "Library/Nerve/NerveActionCtrl.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveStateBase.hpp"
#include "Library/Nerve/NerveStateCtrl.hpp"

namespace al {
/**
 * @brief Changes the current nerve of a nerve user.
 * @param pUser The nerve user.
 * @param pNerve The nerve to switch to.
 */
void setNerve(IUseNerve* pUser, const Nerve* pNerve) {
    pUser->getNerveKeeper()->setNerve(pNerve);
}

/**
 * @brief Changes the current nerve only when the current nerve is at a given step.
 * @param pUser The nerve user.
 * @param pNerve The nerve to switch to.
 * @param step The step at which the nerve is changed.
 */
void setNerveAtStep(IUseNerve* pUser, const Nerve* pNerve, s32 step) {
    if (getNerveStep(pUser) == step) {
        pUser->getNerveKeeper()->setNerve(pNerve);
    }
}

/**
 * @brief Checks whether the current nerve is at a given step.
 * @param pUser The nerve user.
 * @param step The step to compare against.
 * @return True if the current step equals step.
 */
bool isStep(const IUseNerve* pUser, s32 step) {
    return getNerveStep(pUser) == step;
}

/**
 * @brief Checks whether a nerve is the current nerve.
 * @param pUser The nerve user.
 * @param pNerve The nerve to compare against.
 * @return True if pNerve is the current nerve.
 */
bool isNerve(const IUseNerve* pUser, const Nerve* pNerve) {
    return getNerve(pUser) == pNerve;
}

/**
 * @brief Gets the step count of the current nerve.
 * @param pUser The nerve user.
 * @return The current nerve step.
 */
s32 getNerveStep(const IUseNerve* pUser) {
    return pUser->getNerveKeeper()->mNerveStep;
}

/**
 * @brief Gets the current nerve.
 * @param pUser The nerve user.
 * @return The current nerve.
 */
const Nerve* getNerve(const IUseNerve* pUser) {
    return pUser->getNerveKeeper()->getCurrentNerve();
}

/**
 * @brief Checks whether the current nerve is at its first step.
 * @param pUser The nerve user.
 * @return True if the current step is 0.
 */
bool isFirstStep(const IUseNerve* pUser) {
    return getNerveStep(pUser) == 0;
}

/**
 * @brief Checks whether the current step is less than a value.
 * @param pUser The nerve user.
 * @param step The step to compare against.
 * @return True if the current step is less than step.
 */
bool isLessStep(const IUseNerve* pUser, s32 step) {
    return getNerveStep(pUser) < step;
}

/**
 * @brief Checks whether the current step is less than or equal to a value.
 * @param pUser The nerve user.
 * @param step The step to compare against.
 * @return True if the current step is less than or equal to step.
 */
bool isLessEqualStep(const IUseNerve* pUser, s32 step) {
    return getNerveStep(pUser) <= step;
}

/**
 * @brief Checks whether the current step is greater than a value.
 * @param pUser The nerve user.
 * @param step The step to compare against.
 * @return True if the current step is greater than step.
 */
bool isGreaterStep(const IUseNerve* pUser, s32 step) {
    return getNerveStep(pUser) > step;
}

/**
 * @brief Checks whether the current step is greater than or equal to a value.
 * @param pUser The nerve user.
 * @param step The step to compare against.
 * @return True if the current step is greater than or equal to step.
 */
bool isGreaterEqualStep(const IUseNerve* pUser, s32 step) {
    return getNerveStep(pUser) >= step;
}

/**
 * @brief Checks whether the current step lands on a repeating interval.
 * @param pUser The nerve user.
 * @param interval The length of the interval in steps.
 * @param offset The step at which the interval starts.
 * @return True if the step minus offset is a multiple of interval.
 */
bool isIntervalStep(const IUseNerve* pUser, s32 interval, s32 offset) {
    return (getNerveStep(pUser) - offset) % interval == 0;
}

/**
 * @brief Checks whether the current step is inside an "on" period of an alternating on/off interval.
 * @param pUser The nerve user.
 * @param interval The length of each on or off period in steps.
 * @param offset The step at which the first period starts.
 * @return True during the on periods.
 */
bool isIntervalOnOffStep(const IUseNerve* pUser, s32 interval, s32 offset) {
    return ((getNerveStep(pUser) - offset) / interval) % 2 == 0;
}

/**
 * @brief Checks whether the current nerve was just set and has not been executed yet.
 * @param pUser The nerve user.
 * @return True if the current step is negative.
 */
bool isNewNerve(const IUseNerve* pUser) {
    return getNerveStep(pUser) < 0;
}

/**
 * @brief Calculates the progress of the current nerve as a rate.
 * @param pUser The nerve user.
 * @param max The step at which the rate reaches 1.
 * @return The current step divided by max, clamped to [0, 1].
 */
f32 calcNerveRate(const IUseNerve* pUser, s32 max) {
    if (max < 1) {
        return 1.0f;
    }

    f32 step = getNerveStep(pUser);
    return sead::Mathf::clamp(step / max, 0.0f, 1.0f);
}

/**
 * @brief Calculates the progress of the current nerve between two steps as a rate.
 * @param pUser The nerve user.
 * @param min The step at which the rate is 0.
 * @param max The step at which the rate reaches 1.
 * @return The normalized current step, clamped to [0, 1].
 */
f32 calcNerveRate(const IUseNerve* pUser, s32 min, s32 max) {
    f32 rate = normalize(static_cast<f32>(getNerveStep(pUser)), static_cast<f32>(min),
                         static_cast<f32>(max));
    return sead::Mathf::clamp(rate, 0.0f, 1.0f);
}

/**
 * @brief Calculates the progress of the current nerve with an ease-in curve applied.
 * @param pUser The nerve user.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveEaseInRate(const IUseNerve* pUser, s32 max) {
    return easeIn(calcNerveRate(pUser, max));
}

/**
 * @brief Calculates the progress of the current nerve between two steps with an ease-in curve.
 * @param pUser The nerve user.
 * @param min The step at which the rate is 0.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveEaseInRate(const IUseNerve* pUser, s32 min, s32 max) {
    return easeIn(calcNerveRate(pUser, min, max));
}

/**
 * @brief Calculates the progress of the current nerve with an ease-out curve applied.
 * @param pUser The nerve user.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveEaseOutRate(const IUseNerve* pUser, s32 max) {
    return easeOut(calcNerveRate(pUser, max));
}

/**
 * @brief Calculates the progress of the current nerve between two steps with an ease-out curve.
 * @param pUser The nerve user.
 * @param min The step at which the rate is 0.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveEaseOutRate(const IUseNerve* pUser, s32 min, s32 max) {
    return easeOut(calcNerveRate(pUser, min, max));
}

/**
 * @brief Calculates the progress of the current nerve with an ease-in-out curve applied.
 * @param pUser The nerve user.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveEaseInOutRate(const IUseNerve* pUser, s32 max) {
    return easeInOut(calcNerveRate(pUser, max));
}

/**
 * @brief Calculates the progress of the current nerve between two steps with an ease-in-out curve.
 * @param pUser The nerve user.
 * @param min The step at which the rate is 0.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveEaseInOutRate(const IUseNerve* pUser, s32 min, s32 max) {
    return easeInOut(calcNerveRate(pUser, min, max));
}

/**
 * @brief Calculates the progress of the current nerve with a square-in curve applied.
 * @param pUser The nerve user.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveSquareInRate(const IUseNerve* pUser, s32 max) {
    return squareIn(calcNerveRate(pUser, max));
}

/**
 * @brief Calculates the progress of the current nerve between two steps with a square-in curve.
 * @param pUser The nerve user.
 * @param min The step at which the rate is 0.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveSquareInRate(const IUseNerve* pUser, s32 min, s32 max) {
    return squareIn(calcNerveRate(pUser, min, max));
}

/**
 * @brief Calculates the progress of the current nerve with a square-out curve applied.
 * @param pUser The nerve user.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveSquareOutRate(const IUseNerve* pUser, s32 max) {
    return squareOut(calcNerveRate(pUser, max));
}

/**
 * @brief Calculates the progress of the current nerve between two steps with a square-out curve.
 * @param pUser The nerve user.
 * @param min The step at which the rate is 0.
 * @param max The step at which the rate reaches 1.
 * @return The eased rate.
 */
f32 calcNerveSquareOutRate(const IUseNerve* pUser, s32 min, s32 max) {
    return squareOut(calcNerveRate(pUser, min, max));
}

/**
 * @brief Interpolates linearly between two values using the current nerve's progress.
 * @param pUser The nerve user.
 * @param max The step at which the end value is reached.
 * @param start The value at step 0.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveRate(pUser, max), start, end);
}

/**
 * @brief Interpolates linearly between two values using the current nerve's progress between two steps.
 * @param pUser The nerve user.
 * @param min The step at which the start value is used.
 * @param max The step at which the end value is reached.
 * @param start The value at step min.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveRate(pUser, min, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's ease-in progress.
 * @param pUser The nerve user.
 * @param max The step at which the end value is reached.
 * @param start The value at step 0.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveEaseInValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveEaseInRate(pUser, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's ease-in progress between two steps.
 * @param pUser The nerve user.
 * @param min The step at which the start value is used.
 * @param max The step at which the end value is reached.
 * @param start The value at step min.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveEaseInValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveEaseInRate(pUser, min, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's ease-out progress.
 * @param pUser The nerve user.
 * @param max The step at which the end value is reached.
 * @param start The value at step 0.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveEaseOutValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveEaseOutRate(pUser, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's ease-out progress between two steps.
 * @param pUser The nerve user.
 * @param min The step at which the start value is used.
 * @param max The step at which the end value is reached.
 * @param start The value at step min.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveEaseOutValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveEaseOutRate(pUser, min, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's ease-in-out progress.
 * @param pUser The nerve user.
 * @param max The step at which the end value is reached.
 * @param start The value at step 0.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveEaseInOutValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveEaseInOutRate(pUser, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's ease-in-out progress between two steps.
 * @param pUser The nerve user.
 * @param min The step at which the start value is used.
 * @param max The step at which the end value is reached.
 * @param start The value at step min.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveEaseInOutValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveEaseInOutRate(pUser, min, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's square-in progress.
 * @param pUser The nerve user.
 * @param max The step at which the end value is reached.
 * @param start The value at step 0.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveSquareInValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveSquareInRate(pUser, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's square-in progress between two steps.
 * @param pUser The nerve user.
 * @param min The step at which the start value is used.
 * @param max The step at which the end value is reached.
 * @param start The value at step min.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveSquareInValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveSquareInRate(pUser, min, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's square-out progress.
 * @param pUser The nerve user.
 * @param max The step at which the end value is reached.
 * @param start The value at step 0.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveSquareOutValue(const IUseNerve* pUser, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveSquareOutRate(pUser, max), start, end);
}

/**
 * @brief Interpolates between two values using the current nerve's square-out progress between two steps.
 * @param pUser The nerve user.
 * @param min The step at which the start value is used.
 * @param max The step at which the end value is reached.
 * @param start The value at step min.
 * @param end The value at step max.
 * @return The interpolated value.
 */
f32 calcNerveSquareOutValue(const IUseNerve* pUser, s32 min, s32 max, f32 start, f32 end) {
    return lerpValue(calcNerveSquareOutRate(pUser, min, max), start, end);
}

/**
 * @brief Calculates a jump-shaped value: eases out up to a peak, holds it, then eases back to 0.
 * @param pUser The nerve user.
 * @param inMax The number of steps spent rising to the peak.
 * @param upDuration The number of steps the peak is held.
 * @param release The number of steps spent falling back to 0.
 * @param factor The peak value.
 * @return The value for the current step.
 */
f32 calcNerveJumpValue(const IUseNerve* pUser, s32 inMax, s32 upDuration, s32 release,
                       f32 factor) {
    s32 step = getNerveStep(pUser);
    if (step <= inMax) {
        return calcNerveEaseOutRate(pUser, inMax) * factor;
    }

    s32 startRelease = upDuration + inMax;
    if (step <= startRelease) {
        return factor;
    }

    return lerpValue(calcNerveEaseInRate(pUser, startRelease, startRelease + release), factor,
                     0.0f);
}

/**
 * @brief Initializes a nerve state and registers it with the user's state controller.
 * @param pUser The nerve user owning the state.
 * @param pState The state to initialize and add.
 * @param pNerve The nerve that runs the state.
 * @param pName The name of the state.
 */
void initNerveState(IUseNerve* pUser, NerveStateBase* pState, const Nerve* pNerve,
                    const char* pName) {
    pState->init();
    pUser->getNerveKeeper()->mStateCtrl->addState(pState, pNerve, pName);
}

/**
 * @brief Registers a nerve state with the user's state controller.
 * @param pUser The nerve user owning the state.
 * @param pState The state to add.
 * @param pNerve The nerve that runs the state.
 * @param pName The name of the state.
 */
void addNerveState(IUseNerve* pUser, NerveStateBase* pState, const Nerve* pNerve,
                   const char* pName) {
    pUser->getNerveKeeper()->mStateCtrl->addState(pState, pNerve, pName);
}

/**
 * @brief Updates the currently active nerve state.
 * @param pUser The nerve user.
 * @return True if the state has ended.
 */
bool updateNerveState(IUseNerve* pUser) {
    return pUser->getNerveKeeper()->mStateCtrl->updateCurrentState();
}

/**
 * @brief Updates the currently active nerve state and switches nerve once it ends.
 * @param pUser The nerve user.
 * @param pNerve The nerve to switch to once the state ends.
 * @return True if the state ended and the nerve was changed.
 */
bool updateNerveStateAndNextNerve(IUseNerve* pUser, const Nerve* pNerve) {
    if (pUser->getNerveKeeper()->mStateCtrl->updateCurrentState()) {
        pUser->getNerveKeeper()->setNerve(pNerve);
        return true;
    }

    return false;
}

/**
 * @brief Checks whether the currently active nerve state has ended.
 * @param pUser The nerve user.
 * @return True if the current state has ended.
 */
bool isStateEnd(const IUseNerve* pUser) {
    return pUser->getNerveKeeper()->mStateCtrl->isCurrentStateEnd();
}
}  // namespace al

namespace alNerveFunction {
/**
 * @brief Changes the current nerve to the nerve action with a given name.
 * @param pUser The nerve user.
 * @param pActionName The name of the nerve action to switch to.
 */
void setNerveAction(al::IUseNerve* pUser, const char* pActionName) {
    al::NerveKeeper* pKeeper = pUser->getNerveKeeper();
    pKeeper->setNerve(pKeeper->mActionCtrl->findNerve(pActionName));
}
}  // namespace alNerveFunction
