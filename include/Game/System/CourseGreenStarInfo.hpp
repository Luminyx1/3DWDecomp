#pragma once

#include <basis/seadTypes.h>

class CourseGreenStarInfo {
  public:
    CourseGreenStarInfo();
    void initialize();
    void copy(const CourseGreenStarInfo& rOther);
    s32 calcGreenStarAcquireNum(s32 starNum) const;
    bool isCompleteAcquire(s32 starNum) const;

    /**
     * @brief Tests the acquired flag for a single Green Star.
     * @param starIndex Zero-based bit index, from 0 to 31.
     * @return True when the corresponding star has been acquired.
     */
    bool isAcquired(s32 starIndex) const { return ((1u << starIndex) & mFlags) != 0; }

    u32 mFlags;
};

static_assert(sizeof(CourseGreenStarInfo) == 4);
