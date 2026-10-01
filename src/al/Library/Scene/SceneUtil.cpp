#include "Library/Scene/SceneUtil.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Camera/CameraDirector.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/Controller/PadRumbleDirector.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/File/FileUtil.hpp"
#include "Library/HitSensor/HitSensorDirector.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"
#include "Library/Layout/LayoutKit.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Scene/Scene.hpp"
#include "Library/Scene/SceneStopCtrl.hpp"
#include "Library/Screen/ScreenCoverCtrl.hpp"
#include "Library/Sequence/DemoDirector.hpp"
#include "Library/Stage/StageInfo.hpp"
#include "Library/Stage/StageResourceKeeper.hpp"
#include "Library/Stage/StageResourceList.hpp"
#include "Library/System/DrawSystemInfo.hpp"
#include "Library/System/GameSystemInfo.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"
#include "Project/Scene/SceneInitInfo.hpp"

namespace al {
class IAudioResourceLoader;

class ClippingDirectorBase {
public:
    void setClippingJudgeUsClippingPosAsPlayerPos(bool isUse);
};

class CameraResourceHolder {
public:
    CameraResourceHolder(const char* pName, s32 maxResources);
    bool tryInitCameraResource(const Resource* pResource, s32 isOneResource);

    u8 _0[0x18];
};

LiveActor* createPlacementActorFromFactory(const ActorFactory& rFactory,
                                           const ActorInitInfo& rInfo,
                                           const PlacementInfo* pPlacementInfo);
void executeUpdate(LiveActorKit* pKit);
void executeUpdateList(LiveActorKit* pKit, const char* pListName);
void executeUpdateListPaused(LiveActorKit* pKit, const char* pListName);
void executeUpdateListStall(LiveActorKit* pKit, const char* pListName);
void executeUpdate(LayoutKit* pKit);
void executeDraw(const LiveActorKit* pKit, const char* pListName);
void executeDrawList(const LiveActorKit* pKit, const char* pListName, const char* pDrawName);
void executeDraw(const LayoutKit* pKit, const char* pListName);
void updatePadRumbleDirector(LiveActorKit* pKit);
bool tryRequestPreLoadFile(const Resource* pResource, const sead::SafeString& rFileName,
                           sead::Heap* pHeap, IAudioResourceLoader* pLoader);

/**
 * Gets the number of map stage infos.
 * @param pScene scene
 * @return map stage info count
 */
s32 getStageInfoMapNum(const Scene* pScene) {
    return pScene->getStageResourceKeeper()->getMapStageInfo()->getStageResourceNum();
}

/**
 * Gets the number of design stage infos.
 * @param pScene scene
 * @return design stage info count
 */
s32 getStageInfoDesignNum(const Scene* pScene) {
    return pScene->getStageResourceKeeper()->getDesignStageInfo()->getStageResourceNum();
}

/**
 * Gets the number of sound stage infos.
 * @param pScene scene
 * @return sound stage info count
 */
s32 getStageInfoSoundNum(const Scene* pScene) {
    return pScene->getStageResourceKeeper()->getSoundStageInfo()->getStageResourceNum();
}

/**
 * Checks if the stage consists of a single resource.
 * @param pScene scene
 * @return whether the stage is one resource
 */
bool isStageOneResource(const Scene* pScene) {
    return pScene->getStageResourceKeeper()->getMapStageInfo()->isOneResource();
}

/**
 * Gets a map stage info.
 * @param pScene scene
 * @param index stage info index
 * @return stage info
 */
StageInfo* getStageInfoMap(const Scene* pScene, s32 index) {
    return pScene->getStageResourceKeeper()->getMapStageInfo()->getStageInfo(index);
}

/**
 * Gets a design stage info.
 * @param pScene scene
 * @param index stage info index
 * @return stage info
 */
StageInfo* getStageInfoDesign(const Scene* pScene, s32 index) {
    return pScene->getStageResourceKeeper()->getDesignStageInfo()->getStageInfo(index);
}

/**
 * Gets a sound stage info.
 * @param pScene scene
 * @param index stage info index
 * @return stage info
 */
StageInfo* getStageInfoSound(const Scene* pScene, s32 index) {
    return pScene->getStageResourceKeeper()->getSoundStageInfo()->getStageInfo(index);
}

/**
 * Gets the resource of a map stage info.
 * @param pScene scene
 * @param index stage info index
 * @return resource
 */
Resource* getStageResourceMap(const Scene* pScene, s32 index) {
    return getStageInfoMap(pScene, index)->getResource();
}

/**
 * Gets the resource of a design stage info if there is one.
 * @param pScene scene
 * @param index stage info index
 * @return resource, or null
 */
Resource* tryGetStageResourceDesign(const Scene* pScene, s32 index) {
    if (getStageInfoDesignNum(pScene) == 0) {
        return nullptr;
    }

    return getStageInfoDesign(pScene, index)->getResource();
}

/**
 * Gets the resource of a sound stage info if there is one.
 * @param pScene scene
 * @param index stage info index
 * @return resource, or null
 */
Resource* tryGetStageResourceSound(const Scene* pScene, s32 index) {
    if (getStageInfoSoundNum(pScene) == 0) {
        return nullptr;
    }

    return getStageInfoSound(pScene, index)->getResource();
}

/**
 * Finds a stage info by name.
 * @param pScene scene
 * @param listIndex stage resource list index
 * @param pName stage name
 * @return stage info
 */
StageInfo* findStageInfo(const Scene* pScene, s32 listIndex, const char* pName) {
    return pScene->getStageResourceKeeper()->getStageResourceList(listIndex)->findStageInfo(pName);
}

/**
 * Gets the main frame buffer of the scene.
 * @param pScene scene
 * @return frame buffer
 */
sead::FrameBuffer* getSceneFrameBufferMain(const Scene* pScene) {
    DrawSystemInfo* info = pScene->getDrawSystemInfo();

    if (info->mIsDocked) {
        return info->mDockedFrameBuffer;
    }

    return info->mHandheldFrameBuffer;
}

/**
 * Gets the draw context of the scene.
 * @param pScene scene
 * @return draw context
 */
agl::DrawContext* getSceneDrawContext(const Scene* pScene) {
    return pScene->getDrawSystemInfo()->mDrawContext;
}

/**
 * Initializes an actor init info from the scene.
 * @param pInfo actor init info
 * @param pScene scene
 * @param pPlacementInfo placement info
 * @param pLayoutInfo layout init info
 * @param isUseCameraRS whether the RS camera director is used
 */
void initActorInitInfo(ActorInitInfo* pInfo, const Scene* pScene,
                       const PlacementInfo* pPlacementInfo, const LayoutInitInfo* pLayoutInfo,
                       bool isUseCameraRS) {
    LiveActorKit* kit = pScene->getLiveActorKit();
    pInfo->initNew(pPlacementInfo, pLayoutInfo, kit->getExecuteDirector(), pScene->getAudioDirector(),
                   kit->getEffectSystem()->getEffectSystemInfo(), pScene->getOceanWaveDirector(),
                   pScene->getSceneObjHolder(), pScene->getSceneStopCtrl(),
                   pScene->getScreenCoverCtrl(), kit->getHitSensorDirector(), kit->getScreenPointDirector(),
                   kit->getClippingDirector(), kit->getCollisionDirector(), kit->getAreaObjDirector(),
                   kit->getStageSwitchDirector(), kit->getPlayerHolder(), kit->getItemDirector(),
                   kit->getShadowDirector(), kit->getPadRumbleDirector(), kit->getCameraDirector_RS(),
                   kit->getGraphicsSystemInfo(),
                   isUseCameraRS ? kit->getCameraDirector_RS()->getSceneCameraInfo() :
                                   kit->getCameraDirector()->getSceneCameraInfo(),
                   kit->getDemoDirector(), kit->getActorGroup(), isUseCameraRS);
}

/**
 * Initializes a layout init info from the scene.
 * @param pInfo layout init info
 * @param pScene scene
 * @param rInfo scene init info
 */
void initLayoutInitInfo(LayoutInitInfo* pInfo, const Scene* pScene, const SceneInitInfo& rInfo) {
    LiveActorKit* kit = pScene->getLiveActorKit();
    LayoutKit* layoutKit = pScene->getLayoutKit();

    if (kit != nullptr) {
        pInfo->init(kit->getExecuteDirector(), kit->getEffectSystem()->getEffectSystemInfo(),
                    pScene->getSceneObjHolder(), pScene->getAudioDirector(),
                    kit->getCameraDirector(), pScene->getSceneCameraInfo(),
                    rInfo.mGameSystemInfo->getLayoutSystem(),
                    rInfo.mGameSystemInfo->getMessageSystem(),
                    rInfo.mGameSystemInfo->getGamePadSystem(), kit->getPadRumbleDirector());
        if (layoutKit != nullptr) {
            pInfo->setDrawContext(layoutKit->getDrawContext());
            pInfo->setDrawInfo(layoutKit->getDrawInfo());
        }
    } else {
        pInfo->init(layoutKit->getExecuteDirector(), layoutKit->getEffectSystem()->getEffectSystemInfo(),
                    pScene->getSceneObjHolder(), pScene->getAudioDirector(), nullptr,
                    pScene->getSceneCameraInfo(), rInfo.mGameSystemInfo->getLayoutSystem(),
                    rInfo.mGameSystemInfo->getMessageSystem(),
                    rInfo.mGameSystemInfo->getGamePadSystem(), nullptr);
    }
}

namespace {
inline bool tryInitAreaInitInfo(AreaInitInfo* pOut, const Scene* pScene,
                                const ActorInitInfo& rInfo, const StageInfo* pStageInfo) {
    if (!pScene->isValidPlacementParent(pStageInfo->getPlacementInfo())) {
        return false;
    }

    PlacementInfo placementInfo;

    if (!tryGetPlacementInfo(&placementInfo, pStageInfo, "AreaList")) {
        return false;
    }

    pOut->set(placementInfo, rInfo.getStageSwitchDirector());
    return true;
}
}  // namespace

/**
 * Places the area objects of all stage infos.
 * @param pScene scene
 * @param rInfo actor init info
 * @param pExtraList additional stage resource list
 */
void initPlacementAreaObj(Scene* pScene, const ActorInitInfo& rInfo,
                          const StageResourceList* pExtraList) {
    AreaInitInfo infos[512];
    s32 num = 0;

    for (s32 i = 0; i < getStageInfoMapNum(pScene); i++) {
        if (tryInitAreaInitInfo(&infos[num], pScene, rInfo, getStageInfoMap(pScene, i))) {
            num++;
        }
    }

    for (s32 i = 0; i < getStageInfoDesignNum(pScene); i++) {
        if (tryInitAreaInitInfo(&infos[num], pScene, rInfo, getStageInfoDesign(pScene, i))) {
            num++;
        }
    }

    for (s32 i = 0; i < getStageInfoSoundNum(pScene); i++) {
        if (tryInitAreaInitInfo(&infos[num], pScene, rInfo, getStageInfoSound(pScene, i))) {
            num++;
        }
    }

    if (pExtraList != nullptr) {
        s32 extraNum = pExtraList->getStageResourceNum();

        for (s32 i = 0; i < extraNum; i++) {
            if (tryInitAreaInitInfo(&infos[num], pScene, rInfo, pExtraList->getStageInfo(i))) {
                num++;
            }
        }
    }

    pScene->getLiveActorKit()->getAreaObjDirector()->placement(infos, num,
                                                           pScene->getSceneObjHolder(), pScene);
}

/**
 * Places the objects of a map stage info list.
 * @param pScene scene
 * @param rInfo actor init info
 * @param rFactory actor factory
 * @param pName placement list name
 */
void initPlacementObjectMap(Scene* pScene, const ActorInitInfo& rInfo,
                            const ActorFactory& rFactory, const char* pName) {
    s32 num = getStageInfoMapNum(pScene);

    for (s32 i = 0; i < num; i++) {
        initPlacementByStageInfo(getStageInfoMap(pScene, i), pName, rFactory, rInfo);
    }
}

/**
 * Places the objects of a stage info.
 * @param pStageInfo stage info
 * @param pName placement list name
 * @param rFactory actor factory
 * @param rInfo actor init info
 */
void initPlacementByStageInfo(const StageInfo* pStageInfo, const char* pName,
                              const ActorFactory& rFactory, const ActorInitInfo& rInfo) {
    PlacementInfo placementInfo;
    s32 count = 0;

    if (!tryGetPlacementInfoAndCount(&placementInfo, &count, pStageInfo, pName)) {
        return;
    }

    for (s32 i = 0; i < count; i++) {
        PlacementInfo info;
        getPlacementInfoByIndex(&info, placementInfo, i);
        createPlacementActorFromFactory(rFactory, rInfo, &info);
    }
}

/**
 * Places the objects of a design stage info list.
 * @param pScene scene
 * @param rInfo actor init info
 * @param rFactory actor factory
 * @param pName placement list name
 */
void initPlacementObjectDesign(Scene* pScene, const ActorInitInfo& rInfo,
                               const ActorFactory& rFactory, const char* pName) {
    s32 num = getStageInfoDesignNum(pScene);

    for (s32 i = 0; i < num; i++) {
        initPlacementByStageInfo(getStageInfoDesign(pScene, i), pName, rFactory, rInfo);
    }
}

/**
 * Places the objects of a sound stage info list.
 * @param pScene scene
 * @param rInfo actor init info
 * @param rFactory actor factory
 * @param pName placement list name
 */
void initPlacementObjectSound(Scene* pScene, const ActorInitInfo& rInfo,
                              const ActorFactory& rFactory, const char* pName) {
    s32 num = getStageInfoSoundNum(pScene);

    for (s32 i = 0; i < num; i++) {
        initPlacementByStageInfo(getStageInfoSound(pScene, i), pName, rFactory, rInfo);
    }
}

/**
 * Places the objects of a placement list and returns the last created actor.
 * @param pScene scene
 * @param rInfo actor init info
 * @param listIndex stage resource list index
 * @param pName placement list name
 * @param rFactory actor factory
 * @return last created actor, or null
 */
LiveActor* tryInitPlacementSingleObject(Scene* pScene, const ActorInitInfo& rInfo, s32 listIndex,
                                        const char* pName, const ActorFactory& rFactory) {
    StageResourceKeeper* keeper = pScene->getStageResourceKeeper();
    s32 num = keeper->getStageResourceList(listIndex)->getStageResourceNum();
    LiveActor* actor = nullptr;

    for (s32 i = 0; i < num; i++) {
        PlacementInfo placementInfo;
        s32 count = 0;
        tryGetPlacementInfoAndCount(
            &placementInfo, &count,
            pScene->getStageResourceKeeper()->getStageResourceList(listIndex)->getStageInfo(i),
            pName);
        for (s32 j = 0; j < count; j++) {
            PlacementInfo info;
            getPlacementInfoByIndex(&info, placementInfo, j);
            LiveActor* created = createPlacementActorFromFactory(rFactory, rInfo, &info);

            if (created != nullptr) {
                actor = created;
            }
        }
    }

    return actor;
}

/**
 * Gets the placement info and object count of a placement list.
 * @param pOut output placement info
 * @param pCount output object count
 * @param pStageInfo stage info
 * @param pName placement list name
 * @return whether the list exists
 */
bool tryGetPlacementInfoAndCount(PlacementInfo* pOut, s32* pCount, const StageInfo* pStageInfo,
                                 const char* pName) {
    ByamlIter iter;

    if (!pStageInfo->getPlacementIter().tryGetIterByKey(&iter, pName)) {
        *pCount = 0;
        return false;
    }

    pOut->set(iter, pStageInfo->getZoneIter(), pStageInfo->getParentInfo(), pStageInfo->getID());
    *pCount = getCountPlacementInfo(*pOut);
    return true;
}

/**
 * Places the valid objects of a placement list into an actor array.
 * @param pScene scene
 * @param rInfo actor init info
 * @param listIndex stage resource list index
 * @param pName placement list name
 * @param rFactory actor factory
 * @param pActors output actor array
 * @param maxActors maximum actor count
 */
void tryInitPlacementCategory(Scene* pScene, const ActorInitInfo& rInfo, s32 listIndex,
                              const char* pName, const ActorFactory& rFactory,
                              LiveActor** pActors, s32 maxActors) {
    s32 num = pScene->getStageResourceKeeper()->getStageResourceList(listIndex)
                  ->getStageResourceNum();
    s32 actorNum = 0;

    for (s32 i = 0; i < num; i++) {
        StageInfo* stageInfo =
            pScene->getStageResourceKeeper()->getStageResourceList(listIndex)->getStageInfo(i);
        if (!pScene->isValidPlacementParent(stageInfo->getPlacementInfo())) {
            continue;
        }

        PlacementInfo placementInfo;
        s32 count = 0;
        tryGetPlacementInfoAndCount(
            &placementInfo, &count,
            pScene->getStageResourceKeeper()->getStageResourceList(listIndex)->getStageInfo(i),
            pName);
        for (s32 j = 0; j < count; j++) {
            PlacementInfo info;
            getPlacementInfoByIndex(&info, placementInfo, j);

            if (!pScene->isValidPlacement(info)) {
                continue;
            }

            LiveActor* actor = createPlacementActorFromFactory(rFactory, rInfo, &info);

            if (actor == nullptr) {
                continue;
            }

            pActors[actorNum] = actor;
            actorNum++;

            if (actorNum >= maxActors) {
                return;
            }
        }
    }
}

/**
 * Places the objects of a stage info.
 * @param pStageInfo stage info
 * @param pName placement list name
 * @param rFactory actor factory
 * @param rInfo actor init info
 */
void initPlacementByStageInfoSingle(const StageInfo* pStageInfo, const char* pName,
                                    const ActorFactory& rFactory, const ActorInitInfo& rInfo) {
    initPlacementByStageInfo(pStageInfo, pName, rFactory, rInfo);
}

/**
 * Gets a placement info from a byml file of a resource.
 * @param pOut output placement info
 * @param pResource resource
 * @param pFileName byml file name
 * @param pName placement list name
 * @return whether the list exists
 */
bool tryGetPlacementInfo(PlacementInfo* pOut, const Resource* pResource, const char* pFileName,
                         const char* pName) {
    if (pResource == nullptr) {
        return false;
    }

    ByamlIter rootIter(pResource->getByml(pFileName));
    ByamlIter iter;
    bool isExist = rootIter.tryGetIterByKey(&iter, pName);
    pOut->set(iter, ByamlIter(), nullptr, -1);
    return isExist;
}

/**
 * Gets the placement info of a placement list.
 * @param pOut output placement info
 * @param pStageInfo stage info
 * @param pName placement list name
 * @return whether the list exists
 */
bool tryGetPlacementInfo(PlacementInfo* pOut, const StageInfo* pStageInfo, const char* pName) {
    ByamlIter iter;

    if (!pStageInfo->getPlacementIter().tryGetIterByKey(&iter, pName)) {
        return false;
    }

    pOut->set(iter, pStageInfo->getZoneIter(), pStageInfo->getParentInfo(), pStageInfo->getID());
    return true;
}

/**
 * Gets the placement info of a placement list.
 * @param pOut output placement info
 * @param pStageInfo stage info
 * @param pName placement list name
 */
void getPlacementInfo(PlacementInfo* pOut, const StageInfo* pStageInfo, const char* pName) {
    ByamlIter iter;
    pStageInfo->getPlacementIter().tryGetIterByKey(&iter, pName);
    pOut->set(iter, pStageInfo->getZoneIter(), pStageInfo->getParentInfo(), pStageInfo->getID());
}

/**
 * Gets the placement info and object count of a placement list.
 * @param pOut output placement info
 * @param pCount output object count
 * @param pStageInfo stage info
 * @param pName placement list name
 */
void getPlacementInfoAndCount(PlacementInfo* pOut, s32* pCount, const StageInfo* pStageInfo,
                              const char* pName) {
    ByamlIter iter;
    pStageInfo->getPlacementIter().tryGetIterByKey(&iter, pName);
    pOut->set(iter, pStageInfo->getZoneIter(), pStageInfo->getParentInfo(), pStageInfo->getID());
    *pCount = getCountPlacementInfo(*pOut);
}

/**
 * Initializes the area object director.
 * @param pScene scene
 * @param pFactory area object factory
 */
void initAreaObjDirector(Scene* pScene, const AreaObjFactory* pFactory) {
    pScene->getLiveActorKit()->getAreaObjDirector()->init(pFactory);
}

/**
 * Initializes the hit sensor director.
 * @param pScene scene
 */
void initHitSensorDirector(Scene* pScene) {
    pScene->getLiveActorKit()->initHitSensorDirector(1, false);
}

/**
 * Sets the item director.
 * @param pScene scene
 * @param pDirector item director
 */
void initItemDirector(Scene* pScene, ItemDirectorBase* pDirector) {
    pScene->getLiveActorKit()->setItemDirector(pDirector);
}

/**
 * Initializes the camera director.
 * @param pScene scene
 * @param pName camera resource name
 * @param pFactory camera poser factory
 */
void initCameraDirector(const Scene* pScene, const char* pName,
                        const CameraPoserFactory* pFactory) {
    LiveActorKit* kit = pScene->getLiveActorKit();
    kit->getCameraDirector()->init(kit->getPlayerHolder());
}

/**
 * Initializes the RS camera director and its camera resources.
 * @param pScene scene
 * @param pName camera resource name
 * @param pFactory camera poser factory
 * @param pSceneCameraInfo scene camera info
 */
void initCameraDirector_RS(const Scene* pScene, const char* pName,
                           const CameraPoserFactory_RS* pFactory,
                           SceneCameraInfo* pSceneCameraInfo) {
    pScene->getLiveActorKit()->getCameraDirector_RS()->init(pScene->getCameraPoserSceneInfo(), pFactory,
                                                       pSceneCameraInfo);
    CameraDirector_RS* director = pScene->getLiveActorKit()->getCameraDirector_RS();
    CameraResourceHolder* holder = new CameraResourceHolder(pName, getStageInfoMapNum(pScene));

    for (s32 i = 0; i < getStageInfoMapNum(pScene); i++) {
        StageInfo* stageInfo = getStageInfoMap(pScene, i);

        if (pScene->isValidPlacementParent(stageInfo->getPlacementInfo())) {
            holder->tryInitCameraResource(stageInfo->getResource(), isStageOneResource(pScene));
        }
    }

    director->initResourceHolder(holder);
    director->initAreaCameraSwitcherSingle();
}

/**
 * Initializes the camera director without stage resources.
 * @param pScene scene
 * @param pFactory camera poser factory
 */
void initCameraDirectorWithoutStageResource(const Scene* pScene,
                                            const CameraPoserFactory* pFactory) {
    LiveActorKit* kit = pScene->getLiveActorKit();
    kit->getCameraDirector()->init(kit->getPlayerHolder());
}

/**
 * Initializes the camera director with a fixed camera.
 * @param pScene scene
 * @param rPos camera position
 * @param rLookAt camera target
 * @param pFactory camera poser factory
 */
void initCameraDirectorFix(const Scene* pScene, const sead::Vector3f& rPos,
                           const sead::Vector3f& rLookAt, const CameraPoserFactory* pFactory) {
    LiveActorKit* kit = pScene->getLiveActorKit();
    kit->getCameraDirector()->init(kit->getPlayerHolder());
}

/**
 * Does nothing.
 * @param pScene scene
 * @param pViewport viewport
 * @param index camera index
 */
void initCameraAspect(const Scene* pScene, const sead::Viewport* pViewport, s32 index) {}

/**
 * Does nothing.
 * @param pScene scene
 * @param size buffer size
 */
void initFollowCameraLookPointBuffer(const Scene* pScene, s32 size) {}

/**
 * Updates the live actor kit.
 * @param pScene scene
 */
void updateKit(Scene* pScene) {
    executeUpdate(pScene->getLiveActorKit());
}

/**
 * Updates an execute list of the live actor kit.
 * @param pScene scene
 * @param pListName execute list name
 */
void updateKitList(Scene* pScene, const char* pListName) {
    executeUpdateList(pScene->getLiveActorKit(), pListName);
}

/**
 * Updates a paused execute list of the live actor kit.
 * @param pScene scene
 * @param pListName execute list name
 */
void updateKitListPaused(Scene* pScene, const char* pListName) {
    executeUpdateListPaused(pScene->getLiveActorKit(), pListName);
}

/**
 * Updates a stalled execute list of the live actor kit.
 * @param pScene scene
 * @param pListName execute list name
 */
void updateKitListStall(Scene* pScene, const char* pListName) {
    executeUpdateListStall(pScene->getLiveActorKit(), pListName);
}

/**
 * Updates the layout kit.
 * @param pScene scene
 */
void updateLayoutKit(Scene* pScene) {
    executeUpdate(pScene->getLayoutKit());
}

/**
 * Updates the effects.
 * @param pScene scene
 */
void updateEffect(Scene* pScene) {
    alExecuteFunction::updateEffect(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the effect system.
 * @param pScene scene
 */
void updateEffectSystem(Scene* pScene) {
    alExecuteFunction::updateEffectSystem(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the stalled effect system.
 * @param pScene scene
 */
void updateEffectSystemStall(Scene* pScene) {
    alExecuteFunction::updateEffectSystemStall(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the player effects.
 * @param pScene scene
 */
void updateEffectPlayer(Scene* pScene) {
    alExecuteFunction::updateEffectPlayer(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the hit stop effects.
 * @param pScene scene
 */
void updateEffectHitStop(Scene* pScene) {
    alExecuteFunction::updateEffectHitStop(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the demo effects.
 * @param pScene scene
 */
void updateEffectDemo(Scene* pScene) {
    alExecuteFunction::updateEffectDemo(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the layout effects.
 * @param pScene scene
 */
void updateEffectLayout(Scene* pScene) {
    alExecuteFunction::updateEffectLayout(pScene->getLiveActorKit()->getExecuteDirector());
}

/**
 * Updates the pad rumble director if there is one.
 * @param pScene scene
 */
void updatePadRumbleDirector(Scene* pScene) {
    if (pScene != nullptr && pScene->getLiveActorKit() != nullptr) {
        updatePadRumbleDirector(pScene->getLiveActorKit());
    }
}

/**
 * Updates the hit sensor director if there is one.
 * @param pScene scene
 */
void updateHitSensorDirector(Scene* pScene) {
    HitSensorDirector* director = pScene->getLiveActorKit()->getHitSensorDirector();

    if (director != nullptr) {
        director->trueExecute();
    }
}

/**
 * Draws a draw list of the live actor kit.
 * @param pScene scene
 * @param pListName draw list name
 */
void drawKit(const Scene* pScene, const char* pListName) {
    executeDraw(pScene->getLiveActorKit(), pListName);
}

/**
 * Draws a draw list of the live actor kit.
 * @param pScene scene
 * @param pListName draw list name
 * @param pDrawName draw name
 */
void drawKitList(const Scene* pScene, const char* pListName, const char* pDrawName) {
    executeDrawList(pScene->getLiveActorKit(), pListName, pDrawName);
}

/**
 * Draws a draw list of the layout kit.
 * @param pScene scene
 * @param pListName draw list name
 */
void drawLayoutKit(const Scene* pScene, const char* pListName) {
    executeDraw(pScene->getLayoutKit(), pListName);
}

/**
 * Draws the deferred effects with the scene camera.
 * @param pScene scene
 * @param index camera index
 */
void drawEffectDeferred(const Scene* pScene, s32 index) {
    EffectSystem* effectSystem = pScene->getLiveActorKit()->getEffectSystem();
    const IUseCamera* camera = pScene;
    alEffectSystemFunction::drawEffectDeferred(effectSystem, getProjectionMtx(camera),
                                               getCameraViewMtx(camera), getCameraNear(camera),
                                               getCameraFar(camera), getCameraFovyRadian(camera));
}

/**
 * Checks if the scene is stopped.
 * @param pScene scene
 * @return whether the scene is stopped
 */
bool isStopScene(const Scene* pScene) {
    SceneStopCtrl* ctrl = pScene->getSceneStopCtrl();

    if (ctrl->_4 != 0) {
        return false;
    }

    return ctrl->_0 > 0;
}

/**
 * Checks if the camera keeps updating while the scene is stopped.
 * @param pScene scene
 * @return whether the camera is updated
 */
bool isStopAndUpdateCamera(const Scene* pScene) {
    return pScene->getSceneStopCtrl()->_8;
}

/**
 * Checks if the players are stopped with the scene.
 * @param pScene scene
 * @return whether the players are stopped
 */
bool isStopScenePlayers(const Scene* pScene) {
    return pScene->getSceneStopCtrl()->_9;
}

/**
 * Checks if a screen cover capture is requested.
 * @param pScene scene
 * @return whether a capture is requested
 */
bool isRequestCaptureScreenCover(const Scene* pScene) {
    return pScene->getScreenCoverCtrl()->mIsRequestCapture;
}

/**
 * Checks if the screen cover is not drawn.
 * @param pScene scene
 * @return whether the screen cover is off
 */
bool isOffDrawScreenCover(const Scene* pScene) {
    return pScene->getScreenCoverCtrl()->mCoverFrames == 0;
}

/**
 * Resets the screen cover.
 * @param pScene scene
 */
void resetCaptureScreenCover(const Scene* pScene) {
    pScene->getScreenCoverCtrl()->mCoverFrames = -1;
}

/**
 * Checks if a demo is active.
 * @param pScene scene
 * @return whether a demo is active
 */
bool isActiveDemo(const Scene* pScene) {
    return pScene->getDemoDirector()->isActiveDemo();
}

/**
 * Checks if any demo is active.
 * @param pScene scene
 * @return whether any demo is active
 */
bool isAnyActiveDemo(const Scene* pScene) {
    return pScene->getDemoDirector()->isAnyActiveDemo();
}

/**
 * Gets the immediate demo switch flag.
 * @param pScene scene
 * @return immediate demo switch flag
 */
bool getImmediateDemoSwitch(const Scene* pScene) {
    return pScene->getDemoDirector()->isImmediateDemoSwitch();
}

/**
 * Resets the immediate demo switch flag.
 * @param pScene scene
 */
void resetImmediateDemoSwitch(const Scene* pScene) {
    pScene->getDemoDirector()->resetImmediateDemoSwitch();
}

/**
 * Gets the name of the active demo.
 * @param pScene scene
 * @return demo name
 */
const char* getActiveDemoName(const Scene* pScene) {
    return pScene->getDemoDirector()->getActiveDemoName();
}

/**
 * Gets the demo actor list.
 * @param pScene scene
 * @return demo actor list
 */
LiveActor** getDemoActorList(const Scene* pScene) {
    return pScene->getDemoDirector()->getDemoActorList();
}

/**
 * Gets the number of demo actors.
 * @param pScene scene
 * @return demo actor count
 */
s32 getDemoActorNum(const Scene* pScene) {
    return pScene->getDemoDirector()->getDemoActorNum();
}

/**
 * Updates the demo actors.
 * @param pScene scene
 */
void updateDemoActor(const Scene* pScene) {
    pScene->getDemoDirector()->updateDemoActor(nullptr);
}

/**
 * Updates the demo actors and their effects.
 * @param pScene scene
 */
void updateDemoActorWithEffects(const Scene* pScene) {
    EffectSystem* effectSystem = pScene->getLiveActorKit()->getEffectSystem();
    pScene->getDemoDirector()->updateDemoActor(effectSystem);
}

/**
 * Makes clipping use the clipping position as player position.
 * @param pScene scene
 */
void onClippingPosAsPlayerPos(const Scene* pScene) {
    pScene->getLiveActorKit()->getClippingDirector()->setClippingJudgeUsClippingPosAsPlayerPos(true);
}

/**
 * Stops using the clipping position as player position.
 * @param pScene scene
 */
void offClippingPosAsPlayerPos(const Scene* pScene) {
    pScene->getLiveActorKit()->getClippingDirector()->setClippingJudgeUsClippingPosAsPlayerPos(false);
}

/**
 * Initializes the pad rumble director of the scene.
 * @param pScene scene
 * @param rInfo scene init info
 */
void initPadRumble(const Scene* pScene, const SceneInitInfo& rInfo) {
    WaveVibrationHolder* holder = rInfo.mGameSystemInfo->getWaveVibrationHolder();

    if (holder == nullptr) {
        return;
    }

    pScene->getLiveActorKit()->getPadRumbleDirector()->setWaveVibrationHolder(holder);
    alAudioSystemFunction::setPadRumbleDirectorForSe(pScene->getAudioDirector(),
                                                     pScene->getLiveActorKit()->getPadRumbleDirector());
}

/**
 * Stops all pad rumbles.
 * @param pScene scene
 */
void stopPadRumble(const Scene* pScene) {
    LiveActorKit* kit = pScene->getLiveActorKit();

    if (kit != nullptr && kit->getPadRumbleDirector() != nullptr) {
        kit->getPadRumbleDirector()->stopAllRumble();
    }
}

/**
 * Pauses the pad rumbles.
 * @param pScene scene
 */
void pausePadRumble(const Scene* pScene) {
    LiveActorKit* kit = pScene->getLiveActorKit();

    if (kit != nullptr && kit->getPadRumbleDirector() != nullptr) {
        kit->getPadRumbleDirector()->pause();
    }
}

/**
 * Ends the pad rumble pause.
 * @param pScene scene
 */
void endPausePadRumble(const Scene* pScene) {
    LiveActorKit* kit = pScene->getLiveActorKit();

    if (kit != nullptr && kit->getPadRumbleDirector() != nullptr) {
        kit->getPadRumbleDirector()->endPause();
    }
}

/**
 * Pauses the active pad rumbles for a demo.
 * @param pScene scene
 */
void pauseDemoPadRumble(const Scene* pScene) {
    LiveActorKit* kit = pScene->getLiveActorKit();

    if (kit != nullptr && kit->getPadRumbleDirector() != nullptr) {
        kit->getPadRumbleDirector()->pauseActiveRumbles();
    }
}

/**
 * Resumes the active pad rumbles after a demo.
 * @param pScene scene
 */
void endPauseDemoPadRumble(const Scene* pScene) {
    LiveActorKit* kit = pScene->getLiveActorKit();

    if (kit != nullptr && kit->getPadRumbleDirector() != nullptr) {
        kit->getPadRumbleDirector()->resumeActiveRumbles();
    }
}

/**
 * Enables pad rumble.
 * @param pScene scene
 */
void validatePadRumble(Scene* pScene) {
    pScene->getLiveActorKit()->getPadRumbleDirector()->validate();
}

/**
 * Disables pad rumble.
 * @param pScene scene
 */
void invalidatePadRumble(Scene* pScene) {
    pScene->getLiveActorKit()->getPadRumbleDirector()->invalidate();
}

/**
 * Sets the pad rumble power level.
 * @param pScene scene
 * @param level power level
 */
void setPadRumblePowerLevel(Scene* pScene, s32 level) {
    pScene->getLiveActorKit()->getPadRumbleDirector()->setPowerLevel(level);
}

/**
 * Gets the preload file list archive.
 * @return preload archive resource
 */
Resource* getPreLoadFileListArc() {
    return findResource("SystemData/PreLoad");
}

/**
 * Requests preloading the files listed for the stage.
 * @param pScene scene
 * @param rInfo scene init info
 * @param scenarioNo scenario number
 * @param pHeap heap for the files
 * @return whether a request was made
 */
bool tryRequestPreLoadFile(const Scene* pScene, const SceneInitInfo& rInfo, s32 scenarioNo,
                           sead::Heap* pHeap) {
    StringTmp<128> fileName("%s%d", rInfo.mStageName, scenarioNo);
    return tryRequestPreLoadFile(getPreLoadFileListArc(), fileName, pHeap, nullptr);
}
}  // namespace al
