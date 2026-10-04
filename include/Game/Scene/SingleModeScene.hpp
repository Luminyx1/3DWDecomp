#pragma once

#include <basis/seadTypes.h>
#include "Scene/InGameSceneBase.hpp"

namespace al {
class GraphicsSystemInfo;
class ViewRenderer;
}  // namespace al

/**
 * @brief Base scene of Bowser's Fury.
 * @note Only the virtual interface up to isBossScene() is declared; the scene state is not
 *       reconstructed yet and kept as padding.
 */
class SingleModeScene : public InGameSceneBase {
  public:
    explicit SingleModeScene(const char* pName);
    ~SingleModeScene() override;

    void init(const al::SceneInitInfo& rInfo) override;
    void kill() override;
    void control() override;
    bool isValidPlacementParent(const al::PlacementInfo& rInfo) const override;
    bool isValidPlacement(const al::PlacementInfo& rInfo) const override;
    void drawMain_() const override;
    void prepareDestroy() override;
    bool isRestartCheck() const override;
    bool isGoal() const override;
    bool isTriggerPause(s32* pPort) const override;

    virtual bool isScenarioComplete(s32 scenarioNo, s32 shineNum);
    virtual al::ViewRenderer* createViewRenderer(al::GraphicsSystemInfo* pInfo);
    virtual void deleteViewRenderer(al::ViewRenderer* pRenderer);
    virtual bool isChangePhase() const;
    virtual bool allowRestartPoint() const;
    virtual bool isIslandScene() const;
    virtual bool isBossScene() const;

  private:
    u8 mUnknownE8[0x378 - 0xe8]; // Unreconstructed single-mode scene state.
};
static_assert(sizeof(SingleModeScene) == 0x378);
