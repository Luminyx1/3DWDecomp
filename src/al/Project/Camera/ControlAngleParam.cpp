#include "Project/Camera/ControlAngleParam.hpp"

namespace al {
/**
 * Creates the parameters with their default limits and steps.
 */
ControlAngleParam::ControlAngleParam()
    : mIsInvalidControl(false), mIsValid(false), mAngleVLimitMin(10.0f), mAngleVLimitMax(45.0f),
      mAngleHLimitMin(-45.0f), mAngleHLimitMax(45.0f), mAngleVStep(15.0f), mAngleHStep(45.0f) {}
}  // namespace al
