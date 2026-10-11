#pragma once
#include "Library/LiveActor/LiveActor.hpp"

// Only the interface and overall size needed by roads are recovered here.
class CourseSelectNode : public al::LiveActor {
public:
    bool isCurve() const;
private:
    unsigned char _148[0xa8];
};
static_assert(sizeof(CourseSelectNode) == 0x1f0);
