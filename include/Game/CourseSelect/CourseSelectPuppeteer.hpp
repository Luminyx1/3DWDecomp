#pragma once
namespace al { class LiveActor; }
// Interface used by linked-pipe demonstrations.
class CourseSelectPuppeteer {
public:
    void startBindWarpActor(const al::LiveActor*, const al::LiveActor*);
    bool isDemoEnd() const;
    bool isDokanDemoWarpWait() const;
    bool isDokanDemoEndWait() const;
    void startDokanWarp();
    void startDokanEnd();
};
