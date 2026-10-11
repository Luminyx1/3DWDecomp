#pragma once

#include "Library/Scene/ISceneObj.hpp"

namespace al { class IUseSceneObjHolder; class LiveActor; }
class CourseSelectNode;
class ICourseSelectActorController;
class CourseSelectSensor;
class CourseSelectPuppeteerGroup;

class CourseSelectDirector : public al::ISceneObj {
public:
    static CourseSelectDirector* getCourseSelectDirector(const al::IUseSceneObjHolder* pUser);
    const char* getSceneObjName() const override;
    bool isEnableEnterSelectedSensor() const;
    bool isTriggerDecideMainPlayer() const;
    bool isPlayLeaveDemo(int userId) const;
    int tryFindNodeList(CourseSelectNode** ppNodes, int capacity, CourseSelectNode* pStart, CourseSelectNode* pEnd) const;
    CourseSelectNode* tryFindFirstCrossingNode(const CourseSelectNode* pStart, int depth) const;
    void tryResetSelectedActorController(const ICourseSelectActorController* pController);
    CourseSelectNode* getNode(int index) const { return mpNodes[index]; }
    void touchPlayer(CourseSelectSensor* pSensor, al::LiveActor* pPlayer);

    CourseSelectPuppeteerGroup* getPuppeteerGroup() const { return mpPuppeteerGroup; }

private:
    // Scene, layout, actor collections, and selection state; names still unrecovered.
    unsigned char _008[0x148];
    CourseSelectNode** mpNodes;
    unsigned char _158[0x30];
    CourseSelectPuppeteerGroup* mpPuppeteerGroup;
    unsigned char _190[0x40];
};

static_assert(sizeof(CourseSelectDirector) == 0x1d0);
