#pragma once
#include "Library/LiveActor/LiveActor.hpp"

// Layout needed by the pipe holder; the remaining pipe state is not yet recovered.
class CourseSelectDokan : public al::LiveActor {
public:
    void setDstDokan(CourseSelectDokan* pDokan);
    CourseSelectDokan* getDstDokan() const { return mpDstDokan; }
    int getLinkId() const { return mLinkId; }
private:
    CourseSelectDokan* mpDstDokan;
    int mLinkId;
    unsigned char _154[0x54];
};
static_assert(sizeof(CourseSelectDokan) == 0x1a8);
