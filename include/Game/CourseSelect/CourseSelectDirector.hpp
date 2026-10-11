#pragma once

#include "Library/Scene/ISceneObj.hpp"

namespace al { class IUseSceneObjHolder; class LiveActor; }
class CourseSelectSensor;
class CourseSelectPuppeteerGroup;

class CourseSelectDirector : public al::ISceneObj {
public:
    static CourseSelectDirector* getCourseSelectDirector(const al::IUseSceneObjHolder* pUser);
    const char* getSceneObjName() const override;
    bool isEnableEnterSelectedSensor() const;
    bool isTriggerDecideMainPlayer() const;
    void touchPlayer(CourseSelectSensor* pSensor, al::LiveActor* pPlayer);

    CourseSelectPuppeteerGroup* getPuppeteerGroup() const { return mpPuppeteerGroup; }

private:
    // Scene, layout, actor collections, and selection state; names still unrecovered.
    unsigned char _008[0x180];
    CourseSelectPuppeteerGroup* mpPuppeteerGroup;
    unsigned char _190[0x40];
};

static_assert(sizeof(CourseSelectDirector) == 0x1d0);
