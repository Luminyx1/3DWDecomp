#pragma once

#include <prim/seadSafeString.h>

#include "Library/Layout/LayoutSceneInfo.hpp"

namespace agl {
class DrawContext;
}

namespace eui {
class DrawInfoEx;
}

namespace nn::font {
class Rectangle;
}

namespace nn::ui2d {
class DrawInfo;
class GraphicsResource;
}  // namespace nn::ui2d

namespace al {
class AudioDirector;
class CameraDirector;
class CustomTagProcessor;
class EffectSystemInfo;
class ExecuteDirector;
class GamePadSystem;
class LayoutActor;
class LayoutKeeper;
class LayoutSystem;
class MessageSystem;
class PadRumbleDirector;
class Resource;
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

void initLayoutActor(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pLayoutName,
                     const char* pArchiveName = nullptr);
void initLayoutActorLocalized(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                              const char* pLayoutName, const char* pArchiveName = nullptr);
void initLayoutActorUseOtherMessage(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                                    const char* pLayoutName, const char* pArchiveName,
                                    const char* pMessageName);
void initLayoutTextPaneAnimator(LayoutActor* pActor, const char* pPaneName);
void initLayoutTextPaneAnimatorWithShadow(LayoutActor* pActor, const char* pPaneName);
void initLayoutPartsActor(LayoutActor* pActor, LayoutActor* pParentActor,
                          const LayoutInitInfo& rInfo, const char* pPaneName,
                          const char* pLayoutName = nullptr);
void initLayoutPartsActorLocalized(LayoutActor* pActor, LayoutActor* pParentActor,
                                   const LayoutInitInfo& rInfo, const char* pPaneName,
                                   const char* pLayoutName = nullptr);
void initLayoutPartsAudioKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                                const sead::SafeString& rName);

void makeLayoutArchivePath(sead::BufferedSafeString* pOut, const sead::SafeString& rName,
                           bool isLocalized);
void makeLayoutResourcePath(sead::BufferedSafeString* pOut, const sead::SafeString& rName,
                            bool isLocalized);
void initLayoutSceneInfo(LayoutActor* pActor, const LayoutInitInfo& rInfo);
void initLayoutEffectKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName);
void initLayoutSeKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName);
void initLayoutBgmKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName);
void initHitReactionKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                           const Resource* pResource, const char* pName);
void initLayoutMessage(LayoutKeeper* pKeeper, const LayoutInitInfo& rInfo,
                       const Resource* pResource);
void initDrawInfoDefault(nn::ui2d::DrawInfo* pDrawInfo, nn::ui2d::GraphicsResource* pResource);
void initDrawInfo(nn::ui2d::DrawInfo* pDrawInfo, nn::ui2d::GraphicsResource* pResource,
                  const nn::font::Rectangle& rRect);
void initLayoutKeeper(LayoutKeeper* pKeeper, const LayoutInitInfo& rInfo,
                      const Resource* pResource, CustomTagProcessor* pTagProcessor,
                      bool isLocalized);
CustomTagProcessor* createTagProcessor(const LayoutInitInfo& rInfo, const Resource* pResource);
void reallocateTextBoxStringBuffer(LayoutActor* pActor, const Resource* pResource,
                                   const char* pPaneName);
}  // namespace al
