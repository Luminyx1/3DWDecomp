#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>
#include "Library/Draw/GraphicsInitArg.hpp"
#include "Library/Scene/IScenarioCompleteChecker.hpp"
#include "Scene/InGameSceneBase.hpp"

namespace al {
class ActorInitInfo;
class GraphicsSystemInfo;
class ISceneObj;
class LayoutInitInfo;
class StageInfo;
class ViewRenderer;
}  // namespace al

class GameDataHolder;
class PlayerRetargettingSelector;
class SingleModeSceneLayout;

/**
 * @brief Base scene of Bowser's Fury.
 * @note The full virtual interface is declared; most of the scene state is not reconstructed
 *       yet and kept as padding.
 */
class SingleModeScene : public InGameSceneBase,
                        public al::ViewRendererCreator,
                        public al::IScenarioCompleteChecker {
  public:
    explicit SingleModeScene(const char* pName);
    ~SingleModeScene() override;

    void init(const al::SceneInitInfo& rInfo) override;
    void appear() override;
    void kill() override;
    void control() override;
    bool isValidPlacementParent(const al::PlacementInfo& rInfo) const override;
    bool isValidPlacement(const al::PlacementInfo& rInfo) const override;
    void drawMain_() const override;
    void prepareDestroy() override;
    bool isRestartCheck() const override;
    bool isGoal() const override;
    bool isTriggerPause(s32* pPort) const override;

    bool isScenarioComplete(s32 scenarioNo, s32 shineNum) override;
    al::ViewRenderer* createViewRenderer(al::GraphicsSystemInfo* pInfo) override;
    void deleteViewRenderer(al::ViewRenderer* pRenderer) override;
    virtual bool isChangePhase() const;
    virtual bool allowRestartPoint() const;
    virtual bool isIslandScene() const;
    virtual bool isBossScene() const;
    virtual bool isGameEnd() const;
    virtual bool isPhaseEnd() const;
    virtual void handlePhaseEnd();
    virtual void requestStageBgmStart();
    virtual void initIslandDataList();
    virtual void initOceanResourceKeeper();
    virtual void findOceanStageInfo();
    virtual void initOceanScenarios();
    virtual void initIslandMap(const al::LayoutInitInfo& rLayoutInfo,
                               const al::ActorInitInfo& rActorInfo);
    virtual void handleGameOver();
    virtual void preInitPlacement(const al::ActorInitInfo& rInfo);
    virtual bool doZoneIDCheck() const;
    virtual void initAreaObj(const al::ActorInitInfo& rInfo);
    virtual void initPlacement(al::ActorInitInfo& rInfo);
    virtual void initPlacementZoneHolders(const al::ActorInitInfo& rInfo);
    virtual void initPlacementObject(const al::StageInfo* pStageInfo,
                                     const al::ActorInitInfo& rInfo, const char* pListName);
    virtual void initPlacementOceanWater(al::ActorInitInfo& rInfo);
    virtual void initPlacementDisasterModeBowser(const al::ActorInitInfo& rInfo);
    virtual void initPlacementLuckyIsland(const al::ActorInitInfo& rInfo);
    virtual void initPlacementRaidonSurf(const al::StageInfo* pStageInfo,
                                         const al::ActorInitInfo& rInfo);
    virtual void initPlacementIntroCameras(const al::ActorInitInfo& rInfo);
    virtual void initIslandKeeper();
    virtual void endInitIslandKeeper();
    virtual void updateDemoCutsceneAddOn();

    void initPlacementPlayer(const al::StageInfo* pStageInfo, const al::ActorInitInfo& rInfo,
                             PlayerRetargettingSelector* pSelector);
    void exeGameEnd();

  protected:
    sead::FixedSafeString<0x40> mStageName;          // 0xf8
    u8 mUnknown150[0x158 - 0x150];                   // Unreconstructed.
    GameDataHolder* mGameDataHolder;                 // 0x158
    u8 mUnknown160[0x1a8 - 0x160];                   // Unreconstructed.
    al::ISceneObj* mGoalItemHolder;                  // 0x1a8
    u8 mUnknown1B0[0x2c8 - 0x1b0];                   // Unreconstructed.
    SingleModeSceneLayout* mSceneLayout;             // 0x2c8
    u8 mUnknown2D0[0x378 - 0x2d0];                   // Unreconstructed.
};
static_assert(sizeof(SingleModeScene) == 0x378);
