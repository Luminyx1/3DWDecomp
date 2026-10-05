#pragma once

namespace al { class IUseSceneObjHolder; }
class DemoOpeningSwitch;
class DemoTimerStageSwitchController;

class CourseSelectDirector {
public:
    static CourseSelectDirector* tryGetCourseSelectDirector(const al::IUseSceneObjHolder* pUser);
    /** @brief Registers the opening-demo switch actor. @param pSwitch Switch controller. */
    void setDemoOpeningSwitch(DemoOpeningSwitch* pSwitch) { mDemoOpeningSwitch = pSwitch; }
    /** @brief Registers timed demo switches. @param pController Timed switch controller. */
    void setDemoTimerStageSwitchController(DemoTimerStageSwitchController* pController) { mDemoTimerStageSwitchController = pController; }

private:
    unsigned char _0[0x1b8];
    DemoTimerStageSwitchController* mDemoTimerStageSwitchController;
    DemoOpeningSwitch* mDemoOpeningSwitch;
};
