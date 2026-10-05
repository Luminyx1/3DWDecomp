#pragma once

namespace al { class IUseSceneObjHolder; }
class DemoOpeningSwitch;

class CourseSelectDirector {
public:
    static CourseSelectDirector* tryGetCourseSelectDirector(const al::IUseSceneObjHolder* pUser);
    /** @brief Registers the opening-demo switch actor. @param pSwitch Switch controller. */
    void setDemoOpeningSwitch(DemoOpeningSwitch* pSwitch) { mDemoOpeningSwitch = pSwitch; }

private:
    unsigned char _0[0x1c0];
    DemoOpeningSwitch* mDemoOpeningSwitch;
};
