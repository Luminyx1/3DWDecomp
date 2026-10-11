#pragma once

#include "Library/Scene/ISceneObj.hpp"

namespace al { class IUseSceneObjHolder; class LiveActor; }
class CourseSelectNode;
class CourseSelectMiniature;
class CourseSelectWindowHolder;
class ICourseSelectActorController;
class CourseSelectSensor;
class CourseSelectPuppeteerGroup;

class CourseSelectDirector : public al::ISceneObj {
public:
    static CourseSelectDirector* getCourseSelectDirector(const al::IUseSceneObjHolder* pUser);
    CourseSelectWindowHolder* getWindowHolder() const { return mpWindows; }
    CourseSelectMiniature* tryFindMiniatureObj(int courseId) const;
    bool isPlayEntryDemoAny() const;
    void reviveUser(int userId, ICourseSelectActorController* pController, bool isEntryDemo);
    void startOpenRocketDemo();
    bool isEndOpenRocketDemo() const;
    const char* getSceneObjName() const override;
    int getMainPlayerPortNum() const;
    al::LiveActor* getMainPlayerActor() const { return mpMainPlayer; }
    bool isEnableEnterSelectedSensor() const;
    bool isTriggerDecideMainPlayer() const;
    bool isPlayLeaveDemo(int userId) const;
    int tryFindNodeList(CourseSelectNode** ppNodes, int capacity, CourseSelectNode* pStart, CourseSelectNode* pEnd) const;
    CourseSelectNode* tryFindFirstCrossingNode(const CourseSelectNode* pStart, int depth) const;
    void tryResetSelectedActorController(const ICourseSelectActorController* pController);
    CourseSelectNode* getNode(int index) const { return mpNodes[index]; }
    void checkDrcTouch(CourseSelectSensor* pSensor);
    int getCurrentWorldId() const { return mCurrentWorldId; }
    void touchPlayer(CourseSelectSensor* pSensor, al::LiveActor* pPlayer);

    CourseSelectPuppeteerGroup* getPuppeteerGroup() const { return mpPuppeteerGroup; }

private:
    // Scene, layout, actor collections, and selection state; names still unrecovered.
    unsigned char _008[0x10];
    CourseSelectWindowHolder* mpWindows;
    unsigned char _020[0x130];
    CourseSelectNode** mpNodes;
    unsigned char _158[8];
    al::LiveActor* mpMainPlayer;
    unsigned char _168[0x20];
    CourseSelectPuppeteerGroup* mpPuppeteerGroup;
    unsigned char _190[0x38];
    int mCurrentWorldId;
    unsigned char _1cc[4];
};

static_assert(sizeof(CourseSelectDirector) == 0x1d0);
