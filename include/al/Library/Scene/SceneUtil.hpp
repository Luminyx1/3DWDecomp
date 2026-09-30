#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace agl {
class DrawContext;
}

namespace sead {
class FrameBuffer;
class Heap;
class Viewport;
}  // namespace sead

namespace al {
class ActorFactory;
class ActorInitInfo;
class AreaObjFactory;
class CameraPoserFactory;
class CameraPoserFactory_RS;
class ItemDirectorBase;
class LayoutInitInfo;
class LiveActor;
class Resource;
class Scene;
class SceneCameraInfo;
class StageInfo;
class StageResourceList;
struct PlacementInfo;
struct SceneInitInfo;

s32 getStageInfoMapNum(const Scene* pScene);
s32 getStageInfoDesignNum(const Scene* pScene);
s32 getStageInfoSoundNum(const Scene* pScene);
bool isStageOneResource(const Scene* pScene);
StageInfo* getStageInfoMap(const Scene* pScene, s32 index);
StageInfo* getStageInfoDesign(const Scene* pScene, s32 index);
StageInfo* getStageInfoSound(const Scene* pScene, s32 index);
Resource* getStageResourceMap(const Scene* pScene, s32 index);
Resource* tryGetStageResourceDesign(const Scene* pScene, s32 index);
Resource* tryGetStageResourceSound(const Scene* pScene, s32 index);
StageInfo* findStageInfo(const Scene* pScene, s32 listIndex, const char* pName);
sead::FrameBuffer* getSceneFrameBufferMain(const Scene* pScene);
agl::DrawContext* getSceneDrawContext(const Scene* pScene);
void initActorInitInfo(ActorInitInfo* pInfo, const Scene* pScene,
                       const PlacementInfo* pPlacementInfo, const LayoutInitInfo* pLayoutInfo,
                       bool isUseCameraRS);
void initLayoutInitInfo(LayoutInitInfo* pInfo, const Scene* pScene, const SceneInitInfo& rInfo);
void initPlacementAreaObj(Scene* pScene, const ActorInitInfo& rInfo,
                          const StageResourceList* pExtraList);
void initPlacementObjectMap(Scene* pScene, const ActorInitInfo& rInfo,
                            const ActorFactory& rFactory, const char* pName);
void initPlacementByStageInfo(const StageInfo* pStageInfo, const char* pName,
                              const ActorFactory& rFactory, const ActorInitInfo& rInfo);
void initPlacementObjectDesign(Scene* pScene, const ActorInitInfo& rInfo,
                               const ActorFactory& rFactory, const char* pName);
void initPlacementObjectSound(Scene* pScene, const ActorInitInfo& rInfo,
                              const ActorFactory& rFactory, const char* pName);
LiveActor* tryInitPlacementSingleObject(Scene* pScene, const ActorInitInfo& rInfo, s32 listIndex,
                                        const char* pName, const ActorFactory& rFactory);
bool tryGetPlacementInfoAndCount(PlacementInfo* pOut, s32* pCount, const StageInfo* pStageInfo,
                                 const char* pName);
void tryInitPlacementCategory(Scene* pScene, const ActorInitInfo& rInfo, s32 listIndex,
                              const char* pName, const ActorFactory& rFactory,
                              LiveActor** pActors, s32 maxActors);
void initPlacementByStageInfoSingle(const StageInfo* pStageInfo, const char* pName,
                                    const ActorFactory& rFactory, const ActorInitInfo& rInfo);
bool tryGetPlacementInfo(PlacementInfo* pOut, const Resource* pResource, const char* pFileName,
                         const char* pName);
bool tryGetPlacementInfo(PlacementInfo* pOut, const StageInfo* pStageInfo, const char* pName);
void getPlacementInfo(PlacementInfo* pOut, const StageInfo* pStageInfo, const char* pName);
void getPlacementInfoAndCount(PlacementInfo* pOut, s32* pCount, const StageInfo* pStageInfo,
                              const char* pName);
void initAreaObjDirector(Scene* pScene, const AreaObjFactory* pFactory);
void initHitSensorDirector(Scene* pScene);
void initItemDirector(Scene* pScene, ItemDirectorBase* pDirector);
void initCameraDirector(const Scene* pScene, const char* pName,
                        const CameraPoserFactory* pFactory);
void initCameraDirector_RS(const Scene* pScene, const char* pName,
                           const CameraPoserFactory_RS* pFactory,
                           SceneCameraInfo* pSceneCameraInfo);
void initCameraDirectorWithoutStageResource(const Scene* pScene,
                                            const CameraPoserFactory* pFactory);
void initCameraDirectorFix(const Scene* pScene, const sead::Vector3f& rPos,
                           const sead::Vector3f& rLookAt, const CameraPoserFactory* pFactory);
void initCameraAspect(const Scene* pScene, const sead::Viewport* pViewport, s32 index);
void initFollowCameraLookPointBuffer(const Scene* pScene, s32 size);
void updateKit(Scene* pScene);
void updateKitList(Scene* pScene, const char* pListName);
void updateKitListPaused(Scene* pScene, const char* pListName);
void updateKitListStall(Scene* pScene, const char* pListName);
void updateLayoutKit(Scene* pScene);
void updateEffect(Scene* pScene);
void updateEffectSystem(Scene* pScene);
void updateEffectSystemStall(Scene* pScene);
void updateEffectPlayer(Scene* pScene);
void updateEffectHitStop(Scene* pScene);
void updateEffectDemo(Scene* pScene);
void updateEffectLayout(Scene* pScene);
void updatePadRumbleDirector(Scene* pScene);
void updateHitSensorDirector(Scene* pScene);
void drawKit(const Scene* pScene, const char* pListName);
void drawKitList(const Scene* pScene, const char* pListName, const char* pDrawName);
void drawLayoutKit(const Scene* pScene, const char* pListName);
void drawEffectDeferred(const Scene* pScene, s32 index);
bool isStopScene(const Scene* pScene);
bool isStopAndUpdateCamera(const Scene* pScene);
bool isStopScenePlayers(const Scene* pScene);
bool isRequestCaptureScreenCover(const Scene* pScene);
bool isOffDrawScreenCover(const Scene* pScene);
void resetCaptureScreenCover(const Scene* pScene);
bool isActiveDemo(const Scene* pScene);
bool isAnyActiveDemo(const Scene* pScene);
bool getImmediateDemoSwitch(const Scene* pScene);
void resetImmediateDemoSwitch(const Scene* pScene);
const char* getActiveDemoName(const Scene* pScene);
LiveActor** getDemoActorList(const Scene* pScene);
s32 getDemoActorNum(const Scene* pScene);
void updateDemoActor(const Scene* pScene);
void updateDemoActorWithEffects(const Scene* pScene);
void onClippingPosAsPlayerPos(const Scene* pScene);
void offClippingPosAsPlayerPos(const Scene* pScene);
void initPadRumble(const Scene* pScene, const SceneInitInfo& rInfo);
void stopPadRumble(const Scene* pScene);
void pausePadRumble(const Scene* pScene);
void endPausePadRumble(const Scene* pScene);
void pauseDemoPadRumble(const Scene* pScene);
void endPauseDemoPadRumble(const Scene* pScene);
void validatePadRumble(Scene* pScene);
void invalidatePadRumble(Scene* pScene);
void setPadRumblePowerLevel(Scene* pScene, s32 level);
Resource* getPreLoadFileListArc();
bool tryRequestPreLoadFile(const Scene* pScene, const SceneInitInfo& rInfo, s32 scenarioNo,
                           sead::Heap* pHeap);
}  // namespace al
