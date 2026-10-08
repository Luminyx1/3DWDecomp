#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

#include "Library/Scene/ISceneObj.hpp"

namespace al {
class ActorInitInfo;
class ErrorViewer;
class HomeButton;
class IUseSceneObjHolder;
class LayoutInitInfo;
class LiveActor;
class NetworkSystem;
}  // namespace al

class CourseSelectLayout;
class CourseSelectMiniature;
class CourseSelectNode;
class CourseSelectScene;
class CourseSelectWindowHolder;
class DemoOpeningSwitch;
class DemoTimerStageSwitchController;

class CourseSelectDirector : public al::ISceneObj {
public:
    explicit CourseSelectDirector(CourseSelectScene* pScene);

    static CourseSelectDirector* tryGetCourseSelectDirector(const al::IUseSceneObjHolder* pUser);
    /** @brief Registers the opening-demo switch actor. @param pSwitch Switch controller. */
    void setDemoOpeningSwitch(DemoOpeningSwitch* pSwitch) { mDemoOpeningSwitch = pSwitch; }
    /** @brief Registers timed demo switches. @param pController Timed switch controller. */
    void setDemoTimerStageSwitchController(DemoTimerStageSwitchController* pController) { mDemoTimerStageSwitchController = pController; }

    void init(const al::ActorInitInfo& rInfo);
    void createLayout(const al::LayoutInitInfo& rInfo, CourseSelectWindowHolder* pWindowHolder,
                      al::NetworkSystem* pNetworkSystem, al::ErrorViewer* pErrorViewer,
                      al::HomeButton* pHomeButton);
    void initAfterPlacement();
    void connectNodeLink();
    void registerObject(al::LiveActor* pActor, s32 worldId);
    void registerStage(CourseSelectMiniature* pMiniature);
    void setWorldId(s32 worldId);
    void setMainPlayer(al::LiveActor* pPlayer);
    void initPlayerPositionOpening(const char* pName);
    void initPlayerPositionAfterEnding();
    void initPlayerPositionStage(s32 courseId);
    void setPlayerPositionWorldStart(s32 worldId);
    CourseSelectMiniature* tryFindMiniatureObj(s32 courseId) const;
    CourseSelectMiniature* findMiniatureObj(s32 courseId) const;
    CourseSelectMiniature* findKoopaCastle(s32 worldId) const;
    void setAfterOpeningDemo();
    void startDemo(bool isSkipLayout);
    void endDemo(bool isAppearLayout);
    bool isDemo() const;
    bool isPlayPuppeterDemoAll() const;
    bool isPlayPuppeterDemoAny() const;
    bool isPlayEntryDemoAny() const;
    bool isPlayLeaveDemo(s32 userId) const;
    bool isInCourseSelectBubbleAny() const;
    void activateUser(s32 userId, bool isAppearDemo);
    void leaveUser(s32 userId);
    void cancelEnter();
    void appearLayout();
    void prepareFrame();
    void update();
    void updateActiveWorld(const sead::BitFlag32& rWorldFlag);

    /** @brief Gets the course select layouts. @return The layout holder. */
    CourseSelectLayout* getLayout() const { return mLayout; }
    /** @brief Gets the course select nodes. @return The node array. */
    CourseSelectNode** getNodes() const { return mNodes; }
    /** @brief Gets the number of course select nodes. @return The node count. */
    s32 getNodeNum() const { return mNodeNum; }
    /** @brief Gets the opening-demo switch actor. @return The switch actor, or nullptr. */
    DemoOpeningSwitch* getDemoOpeningSwitch() const { return mDemoOpeningSwitch; }
    /** @brief Gets the world the main player is in. @return The world id, or -1. */
    s32 getActiveWorldId() const { return mActiveWorldId; }
    /** @brief Gets whether the event gate keeper is talking. @return true while it talks. */
    bool isEventGateKeeper() const { return mIsEventGateKeeper; }

private:
    unsigned char _8[0x10 - 0x8];
    CourseSelectLayout* mLayout;  // 0x10
    unsigned char _18[0x150 - 0x18];
    CourseSelectNode** mNodes;  // 0x150
    s32 mNodeNum;  // 0x158
    unsigned char _15c[0x1b8 - 0x15c];
    DemoTimerStageSwitchController* mDemoTimerStageSwitchController;
    DemoOpeningSwitch* mDemoOpeningSwitch;
    s32 mActiveWorldId;  // 0x1c8
    bool mIsEventGateKeeper;  // 0x1cc
};

static_assert(sizeof(CourseSelectDirector) == 0x1d0);
