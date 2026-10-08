#pragma once

#include <basis/seadTypes.h>

namespace al {
class HitSensor;
}  // namespace al

class ICourseSelectActorController;

/** @brief Kinds of course-select sensors. */
enum CourseSelectSensorType : s64 {
    cCourseSelectSensorType_Actor = 0,
};

/**
 * @brief Links a hit sensor of a course-select object to its controller, so the director knows
 * which object the players touch.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CourseSelectSensor {
public:
    CourseSelectSensor(CourseSelectSensorType type, ICourseSelectActorController* pController);

    void setSensor(const al::HitSensor* pSensor);

private:
    u8 _0[0x18];
};

static_assert(sizeof(CourseSelectSensor) == 0x18);
