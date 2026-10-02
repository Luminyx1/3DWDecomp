#pragma once

#include <prim/seadSafeString.h>

#include "Library/Layout/LayoutInitFunction.hpp"
#include "Library/Layout/LayoutSceneInfo.hpp"

namespace agl {
class DrawContext;
}

namespace eui {
class DrawInfoEx;
}

namespace al {
class AudioDirector;
class CameraDirector;
class EffectSystemInfo;
class ExecuteDirector;
class GamePadSystem;
class LayoutActor;
class LayoutSystem;
class MessageSystem;
class PadRumbleDirector;
class SceneCameraInfo;
class SceneObjHolder;

class LayoutInitInfo : public LayoutSceneInfo {
public:
    LayoutInitInfo();

    void init(ExecuteDirector* pExecuteDirector, const EffectSystemInfo* pEffectSystemInfo,
              SceneObjHolder* pSceneObjHolder, const AudioDirector* pAudioDirector,
              CameraDirector* pCameraDirector, SceneCameraInfo* pSceneCameraInfo,
              const LayoutSystem* pLayoutSystem, const MessageSystem* pMessageSystem,
              const GamePadSystem* pGamePadSystem, PadRumbleDirector* pPadRumbleDirector);
    const MessageSystem* getMessageSystem() const;

    ExecuteDirector* getExecuteDirector() const { return mExecuteDirector; }
    const EffectSystemInfo* getEffectSystemInfo() const { return mEffectSystemInfo; }
    const AudioDirector* getAudioDirector() const { return mAudioDirector; }
    const LayoutSystem* getLayoutSystem() const { return mLayoutSystem; }
    agl::DrawContext* getDrawContext() const { return mDrawContext; }
    eui::DrawInfoEx* getDrawInfo() const { return mDrawInfo; }

    void setDrawContext(agl::DrawContext* pDrawContext) { mDrawContext = pDrawContext; }
    void setDrawInfo(eui::DrawInfoEx* pDrawInfo) { mDrawInfo = pDrawInfo; }

    agl::DrawContext* mDrawContext = nullptr;
    eui::DrawInfoEx* mDrawInfo = nullptr;
    void* _48 = nullptr;
    ExecuteDirector* mExecuteDirector = nullptr;
    const EffectSystemInfo* mEffectSystemInfo = nullptr;
    const AudioDirector* mAudioDirector = nullptr;
    const LayoutSystem* mLayoutSystem = nullptr;
};

void initLayoutActor(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pArchiveName,
                     const char* pSuffix = nullptr);
void initLayoutActorLocalized(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                              const char* pArchiveName, const char* pSuffix = nullptr);
void initLayoutActorUseOtherMessage(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                                    const char* pArchiveName, const char* pMessageArchiveName,
                                    const char* pSuffix);
void initLayoutTextPaneAnimator(LayoutActor* pActor, const char* pPaneName);
void initLayoutTextPaneAnimatorWithShadow(LayoutActor* pActor, const char* pPaneName);
void initLayoutPartsActor(LayoutActor* pActor, LayoutActor* pParentActor,
                          const LayoutInitInfo& rInfo, const char* pPaneName,
                          const char* pSuffix = nullptr);
void initLayoutPartsActorLocalized(LayoutActor* pActor, LayoutActor* pParentActor,
                                   const LayoutInitInfo& rInfo, const char* pPaneName,
                                   const char* pSuffix = nullptr);
void initLayoutPartsAudioKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                                const sead::SafeString& rName);

}  // namespace al
