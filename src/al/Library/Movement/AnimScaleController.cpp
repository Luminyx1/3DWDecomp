#include "Library/Movement/AnimScaleController.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(AnimScaleController, Stop)
NERVE_DECL(AnimScaleController, Anim)
NERVE_DECL(AnimScaleController, Vibration)
NERVE_DECL(AnimScaleController, HitReaction)
NERVE_DECL(AnimScaleController, Crush)

NERVES_MAKE_NOSTRUCT(AnimScaleController, Stop, Anim, Vibration, HitReaction, Crush)
}  // namespace

namespace al {

/**
 * Constructs the default squash and stretch parameters.
 */
AnimScaleParam::AnimScaleParam() = default;

/**
 * Constructs squash and stretch parameters.
 * @param a Stiffness of the regular animation.
 * @param b Damping of the regular animation.
 * @param c Minimum Y scale.
 * @param d Maximum Y scale.
 * @param e Initial velocity of the hit reaction.
 * @param f Stiffness of the hit reaction.
 * @param g Damping of the hit reaction.
 * @param h Length of the crush animation in steps.
 * @param i Strength of the crush animation.
 * @param j Base Y scale of the vibration.
 * @param k Cycle of the vibration in steps.
 * @param l Strength of the vibration.
 */
AnimScaleParam::AnimScaleParam(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, s32 h, f32 i, f32 j,
                               f32 k, f32 l)
    : _0(a), _4(b), _8(c), _c(d), _10(e), _14(f), _18(g), _1c(h), _20(i), _24(j), _28(k), _2c(l) {}

namespace {
const AnimScaleParam sDefaultParam;
}  // namespace

/**
 * Constructs a scale animation controller.
 * @param pParam Animation parameters, or nullptr for the defaults.
 */
AnimScaleController::AnimScaleController(const AnimScaleParam* pParam)
    : NerveExecutor("スケールアニメコントロール"), mParam(pParam) {
    if (pParam == nullptr) {
        mParam = &sDefaultParam;
    }

    initNerve(&NrvAnimScaleControllerStop, 0);
}

/**
 * Replaces the animation parameters.
 * @param pParam New parameters, or nullptr for the defaults.
 */
void AnimScaleController::setAnimScaleParam(const AnimScaleParam* pParam) {
    mParam = (pParam != nullptr) ? pParam : &sDefaultParam;
}

/**
 * Starts the spring animation.
 */
void AnimScaleController::startAnim() {
    setNerve(this, &NrvAnimScaleControllerAnim);
}

/**
 * Starts the vibration animation.
 */
void AnimScaleController::startVibration() {
    setNerve(this, &NrvAnimScaleControllerVibration);
}

/**
 * Starts the hit reaction animation.
 */
void AnimScaleController::startHitReaction() {
    setNerve(this, &NrvAnimScaleControllerHitReaction);
}

/**
 * Sets the Y scale velocity and starts the spring animation.
 * @param velocity New velocity.
 */
void AnimScaleController::startAndSetScaleVelocityY(f32 velocity) {
    mScaleVelocityY = velocity;
    setNerve(this, &NrvAnimScaleControllerAnim);
}

/**
 * Adds to the Y scale velocity and starts the spring animation.
 * @param velocity Amount to add.
 */
void AnimScaleController::startAndAddScaleVelocityY(f32 velocity) {
    mScaleVelocityY += velocity;
    setNerve(this, &NrvAnimScaleControllerAnim);
}

/**
 * Starts the crush animation.
 */
void AnimScaleController::startCrush() {
    setNerve(this, &NrvAnimScaleControllerCrush);
}

/**
 * Stops the animation, keeping the current scale.
 */
void AnimScaleController::stopAnim() {
    mScaleVelocityY = 0.0f;
    setNerve(this, &NrvAnimScaleControllerStop);
}

/**
 * Resets the scale and stops the animation.
 */
void AnimScaleController::stopAndReset() {
    mAnimScale.set(1.0f, 1.0f, 1.0f);
    mScale = mOriginalScale;
    stopAnim();
}

/**
 * Resets the animated scale to the original scale.
 */
void AnimScaleController::resetScale() {
    mScaleVelocityY = 0.0f;
    mAnimScale.set(1.0f, 1.0f, 1.0f);
    mScale = mOriginalScale;
}

/**
 * Stops the animation and sets the animated scale.
 * @param rScale Animated scale to apply on top of the original scale.
 */
void AnimScaleController::stopAndSetScale(const sead::Vector3f& rScale) {
    mScaleVelocityY = 0.0f;
    setNerve(this, &NrvAnimScaleControllerStop);
    mAnimScale.set(rScale);
    mScale.x = mAnimScale.x * mOriginalScale.x;
    mScale.y = mAnimScale.y * mOriginalScale.y;
    mScale.z = mAnimScale.z * mOriginalScale.z;
}

/**
 * Sets the Y scale velocity.
 * @param velocity New velocity.
 */
void AnimScaleController::setScaleVelocityY(f32 velocity) {
    mScaleVelocityY = velocity;
}

/**
 * Adds to the Y scale velocity.
 * @param velocity Amount to add.
 */
void AnimScaleController::addScaleVelocityY(f32 velocity) {
    mScaleVelocityY += velocity;
}

/**
 * Does nothing while stopped.
 */
void AnimScaleController::exeStop() {}

/**
 * Derives the X and Z scale from the Y scale so the volume is preserved, then applies the
 * original scale.
 */
inline void AnimScaleController::updateScaleXZ() {
    f32 scaleXZ = sead::Mathf::sqrt(1.0f / mAnimScale.y);
    mAnimScale.x = scaleXZ;
    mAnimScale.z = scaleXZ;
    mScale.x = mAnimScale.x * mOriginalScale.x;
    mScale.y = mAnimScale.y * mOriginalScale.y;
    mScale.z = mAnimScale.z * mOriginalScale.z;
}

/**
 * Runs the spring animation until it settles.
 */
void AnimScaleController::exeAnim() {
    updateScale(mParam->_0, mParam->_4);
    tryStop();
}

/**
 * Steps the spring towards a Y scale of 1.
 * @param stiffness Spring stiffness.
 * @param damping Velocity damping factor.
 */
void AnimScaleController::updateScale(f32 stiffness, f32 damping) {
    f32 scaleY = mAnimScale.y;
    mScaleVelocityY += (1.0f - scaleY) * stiffness;
    mScaleVelocityY *= damping;
    mAnimScale.y = scaleY + mScaleVelocityY;
    mAnimScale.y = sead::Mathf::clamp(mAnimScale.y, mParam->_8, mParam->_c);
    updateScaleXZ();
}

/**
 * Stops the animation once the spring has settled.
 * @return Whether the animation was stopped.
 */
bool AnimScaleController::tryStop() {
    if (sead::Mathf::abs(1.0f - mAnimScale.y) < 0.001f &&
        sead::Mathf::abs(mScaleVelocityY) < 0.001f) {
        resetScale();
        setNerve(this, &NrvAnimScaleControllerStop);
        return true;
    }

    return false;
}

/**
 * Vibrates the Y scale around its base value.
 */
void AnimScaleController::exeVibration() {
    f32 base = mParam->_24;
    f32 rate = getNerveStep(this) / mParam->_28;
    f32 scaleY = base + sead::Mathf::sin(rate * 2 * sead::Mathf::pi()) * mParam->_2c;
    mAnimScale.y = scaleY < 0.0001f ? 0.0001f : scaleY;  // not sead::Mathf::max (fmaxnm)
    updateScaleXZ();
}

/**
 * Runs the hit reaction spring animation until it settles.
 */
void AnimScaleController::exeHitReaction() {
    if (isFirstStep(this)) {
        resetScale();
        mScaleVelocityY = mParam->_10;
    }

    updateScale(mParam->_14, mParam->_18);
    tryStop();
}

/**
 * Runs the crush animation for a fixed number of steps.
 */
void AnimScaleController::exeCrush() {
    mScaleVelocityY = 0.0f;
    f32 rate = calcNerveRate(this, mParam->_1c);
    mAnimScale.y = calcConvergeVibrationValue(rate, 1.0f, mParam->_20, 0.3f, 4.0f);
    updateScaleXZ();

    if (isGreaterStep(this, mParam->_1c)) {
        setNerve(this, &NrvAnimScaleControllerStop);
    }
}

/**
 * Checks whether the hit reaction is playing and still within a step count.
 * @param step Step limit, or less than 1 for no limit.
 * @return Whether the hit reaction is active.
 */
bool AnimScaleController::isHitReaction(s32 step) const {
    if (!isNerve(this, &NrvAnimScaleControllerHitReaction)) {
        return false;
    }

    if (step < 1) {
        return true;
    }

    return isLessStep(this, step);
}

/**
 * Sets the base scale the animation is applied to.
 * @param rScale Original scale.
 */
void AnimScaleController::setOriginalScale(const sead::Vector3f& rScale) {
    mOriginalScale = rScale;
}

/**
 * Updates the controller's nerve.
 */
void AnimScaleController::update() {
    updateNerve();
}

}  // namespace al
