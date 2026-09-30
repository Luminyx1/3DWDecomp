#include "Library/LiveActor/Util/ActorInitUtil.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Actor/ActorPoseKeeper.hpp"
#include "Library/Audio/AudioDirector.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Collision/CollisionDirector.hpp"
#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/Item/ItemUtil.hpp"
#include "Library/LiveActor/Common/LiveActorGroup.hpp"
#include "Library/LiveActor/HitReactionKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Shadow/ShadowKeeper.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"
#include "Project/Light/ActorPrePassLightKeeper.hpp"
#include "Project/OceanWave/OceanWaveKeeper.hpp"
#include "Project/Play/Actor/ActorAlphaCtrl.hpp"

namespace al {
namespace {
void setupModelKeeper(LiveActor* pActor, ModelKeeper* pModelKeeper, const ActorInitInfo& rInfo,
                      s32 bufferNum) {
    SceneCameraInfo* cameraInfo = rInfo.mActorSceneInfo.sceneCameraInfo;
    pModelKeeper->initModel(
        bufferNum, static_cast<GraphicsSystemInfo*>(rInfo.mActorSceneInfo._78)->mGpuMemAllocator);
    pModelKeeper->mModelCafe->setCameraInfo(
        cameraInfo->mViewMtx, static_cast<const sead::Matrix34f*>(cameraInfo->_8),
        static_cast<const sead::Matrix44f*>(cameraInfo->_10),
        static_cast<const sead::Matrix44f*>(cameraInfo->_18));
    const char* cubeMapName = nullptr;
    tryGetStringArg(&cubeMapName, rInfo, "CubeMapUnitName");
    if (cubeMapName) {
        forceApplyCubeMap(pModelKeeper,
                          static_cast<const GraphicsSystemInfo*>(pActor->getSceneInfo()->_78),
                          cubeMapName);
    }

    const char* cubeMap2Name = nullptr;
    tryGetStringArg(&cubeMap2Name, rInfo, "CubeMap2UnitName");
    pActor->initModelKeeper(pModelKeeper);
}
}  // namespace

/**
 * Creates the scene info of an actor and registers it to the actor group.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActorSceneInfo(LiveActor* pActor, const ActorInitInfo& rInfo) {
    ActorSceneInfo* sceneInfo = new ActorSceneInfo();
    *sceneInfo = rInfo.mActorSceneInfo;
    pActor->initSceneInfo(sceneInfo);
    rInfo.mLiveActorGroup->registerActor(pActor);
    pActor->setPlacementHolder(rInfo);
}

/**
 * Registers an actor to an update execute list.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pListName The list name.
 */
void initExecutorUpdate(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pListName) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, pListName);
}

/**
 * Registers an actor to a draw execute list.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pListName The list name.
 */
void initExecutorDraw(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pListName) {
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, pListName);
}

/**
 * Registers an actor to the "プレイヤー" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorPlayer(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "プレイヤー");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "プレイヤー");
}

/**
 * Registers an actor to the "プレイヤー[PreMovement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorPlayerPreMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "プレイヤー[PreMovement]");
}

/**
 * Registers an actor to the "プレイヤー[Movement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorPlayerMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "プレイヤー[Movement]");
}

/**
 * Registers an actor to the "プレイヤーモデル" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorPlayerModel(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "プレイヤーモデル");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "プレイヤーモデル");
}

/**
 * Registers an actor to the "プレイヤー装飾" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorPlayerDecoration(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "プレイヤー装飾");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "プレイヤー装飾");
}

/**
 * Registers an actor to the "敵" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorEnemy(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "敵");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "敵");
}

/**
 * Registers an actor to the "敵[Movement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorEnemyMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "敵[Movement]");
}

/**
 * Registers an actor to the "敵装飾" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorEnemyDecoration(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "敵装飾");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "敵装飾");
}

/**
 * Registers an actor to the "敵装飾[Movement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorEnemyDecorationMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "敵装飾[Movement]");
}

/**
 * Registers an actor to the "EnemyMapObj[Movement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorEnemyMapObjMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "EnemyMapObj[Movement]");
}

/**
 * Registers an actor to the "地形オブジェ" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorMapObj(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "地形オブジェ");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "地形オブジェ");
}

/**
 * Registers an actor to the "地形オブジェ[Movement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorMapObjMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "地形オブジェ[Movement]");
}

/**
 * Registers an actor to the "地形オブジェ装飾" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorMapObjDecoration(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "地形オブジェ装飾");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "地形オブジェ装飾");
}

/**
 * Registers an actor to the "影ボリューム" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorShadowVolume(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "影ボリューム");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "影ボリューム");
}

/**
 * Registers an actor to the "影ボリュームのフィル" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorShadowVolumeFillStencil(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "影ボリュームのフィル");
    registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "影ボリュームのフィル");
}

/**
 * Registers an actor to the "コリジョン地形装飾[Movement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorCollisionMapObjDecorationMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "コリジョン地形装飾[Movement]");
}

/**
 * Registers an actor to the "監視オブジェ" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorWatchObj(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "監視オブジェ");
}

/**
 * Registers an actor to the "デバッグ[ActorMovement]" execute lists.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initExecutorDebugMovement(LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerExecutorActorUpdate(pActor, rInfo.mExecuteDirector, "デバッグ[ActorMovement]");
}

/**
 * Creates a TRSV pose keeper for an actor.
 * @param pActor The actor.
 */
void initActorPoseTRSV(LiveActor* pActor) {
    pActor->initPoseKeeper(new ActorPoseKeeperTRSV());
}

/**
 * Creates a TRMSV pose keeper for an actor.
 * @param pActor The actor.
 */
void initActorPoseTRMSV(LiveActor* pActor) {
    pActor->initPoseKeeper(new ActorPoseKeeperTRMSV());
}

/**
 * Creates a TFSV pose keeper for an actor.
 * @param pActor The actor.
 */
void initActorPoseTFSV(LiveActor* pActor) {
    pActor->initPoseKeeper(new ActorPoseKeeperTFSV());
}

/**
 * Creates a TFGSV pose keeper for an actor.
 * @param pActor The actor.
 */
void initActorPoseTFGSV(LiveActor* pActor) {
    pActor->initPoseKeeper(new ActorPoseKeeperTFGSV());
}

/**
 * Creates a TQSV pose keeper for an actor.
 * @param pActor The actor.
 */
void initActorPoseTQSV(LiveActor* pActor) {
    pActor->initPoseKeeper(new ActorPoseKeeperTQSV());
}

/**
 * Sets the translation, rotation and scale of an actor from its placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActorSRT(LiveActor* pActor, const ActorInitInfo& rInfo) {
    if (!pActor->mActorPoseKeeper) {
        initActorPoseTRSV(pActor);
    }

    sead::Vector3f trans = {0.0f, 0.0f, 0.0f};
    tryGetTrans(&trans, rInfo);
    setTrans(pActor, trans);
    sead::Vector3f rotate = {0.0f, 0.0f, 0.0f};
    tryGetRotate(&rotate, rInfo);
    updatePoseRotate(pActor, rotate);
    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    tryGetScale(&scale, rInfo);
    setScale(pActor, scale);
}

/**
 * Sets the translation, rotation and scale of an actor from its placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActorSRT_ParentY(LiveActor* pActor, const ActorInitInfo& rInfo) {
    if (!pActor->mActorPoseKeeper) {
        initActorPoseTRSV(pActor);
    }

    sead::Vector3f trans = {0.0f, 0.0f, 0.0f};
    tryGetTrans(&trans, rInfo);
    setTrans(pActor, trans);
    sead::Vector3f rotate = {0.0f, 0.0f, 0.0f};
    tryGetRotate_ParentY(&rotate, rInfo);
    updatePoseRotate(pActor, rotate);
    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    tryGetScale(&scale, rInfo);
    setScale(pActor, scale);
}

/**
 * Creates and initializes a model keeper for an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The model archive name.
 * @param bufferNum The number of model buffers.
 * @param pAnimArchiveName The animation archive name.
 */
void initActorModelKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName, s32 bufferNum, const char* pAnimArchiveName) {
    ModelKeeper* modelKeeper = new ModelKeeper();
    modelKeeper->initResource(pArchiveName, pAnimArchiveName, nullptr);
    setupModelKeeper(pActor, modelKeeper, rInfo, bufferNum);
    sead::Matrix34f baseMtx;
    pActor->mActorPoseKeeper->calcBaseMtx(&baseMtx);
    setBaseMtxAndCalcAnim(pActor, baseMtx, pActor->mActorPoseKeeper->getScale());
}

/**
 * Creates and initializes a model keeper for an actor using an init file.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rIter The init file.
 * @param pArchiveName The model archive name.
 * @param bufferNum The number of model buffers.
 * @param pAnimArchiveName The animation archive name.
 * @param pSuffix The file suffix.
 */
void initActorModelKeeperWithInitFile(LiveActor* pActor, const ActorInitInfo& rInfo, const ByamlIter& rIter, const char* pArchiveName, s32 bufferNum, const char* pAnimArchiveName, const char* pSuffix) {
    ModelKeeper* modelKeeper = new ModelKeeper();
    modelKeeper->initResource(pArchiveName, pAnimArchiveName, pSuffix);
    setupModelKeeper(pActor, modelKeeper, rInfo, bufferNum);
    sead::Matrix34f baseMtx;
    pActor->mActorPoseKeeper->calcBaseMtx(&baseMtx);
    setBaseMtxAndCalcAnim(pActor, baseMtx, pActor->mActorPoseKeeper->getScale());
    modelKeeper->mModelCafe->initUpdateBounding();
}

/**
 * Creates and initializes a shadow volume model keeper for an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The model archive name.
 */
void initActorModelKeeperShadowVolume(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName) {
    ModelKeeper* modelKeeper = new ModelKeeper();
    modelKeeper->initResource(pArchiveName, nullptr, nullptr);
    setupModelKeeper(pActor, modelKeeper, rInfo, 1);
    sead::Matrix34f baseMtx;
    pActor->mActorPoseKeeper->calcBaseMtx(&baseMtx);
    setBaseMtxAndCalcAnim(pActor, baseMtx, pActor->mActorPoseKeeper->getScale());
}

/**
 * Creates and initializes a shadow volume model keeper for an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The model archive name.
 */
void initActorModelKeeperShadowVolumeFillStencil(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName) {
    ModelKeeper* modelKeeper = new ModelKeeper();
    modelKeeper->initResource(pArchiveName, nullptr, nullptr);
    setupModelKeeper(pActor, modelKeeper, rInfo, 1);
    sead::Matrix34f baseMtx;
    pActor->mActorPoseKeeper->calcBaseMtx(&baseMtx);
    setBaseMtxAndCalcAnim(pActor, baseMtx, pActor->mActorPoseKeeper->getScale());
}

/**
 * Applies the cube map named in the placement of an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActorModelForceCubeMap(LiveActor* pActor, const ActorInitInfo& rInfo) {
    const char* cubeMapName = nullptr;
    tryGetStringArg(&cubeMapName, rInfo, "CubeMapUnitName");
    if (cubeMapName && pActor->mModelKeeper) {
        forceApplyCubeMap(pActor, cubeMapName);
    }
}

/**
 * Creates the effect keeper of an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The effect user name.
 * @param isUnused Unused.
 */
void initActorEffectKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, bool isUnused) {
    EffectKeeper* effectKeeper = new EffectKeeper(rInfo.mEffectSystemInfo, pName, getTransPtr(pActor), nullptr, pActor->getBaseMtx());
    pActor->initEffectKeeper(effectKeeper);
}

/**
 * Creates the audio keeper of an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pSeName The sound effect user name.
 * @param pBgmName The music user name, or nullptr to use the sound effect name.
 */
void initActorAudioKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pSeName, const char* pBgmName) {
    AudioKeeper* audioKeeper = new AudioKeeper();
    const AudioDirector* audioDirector = rInfo.mAudioDirector;
    if (audioDirector->isForceInvalidSe()) {
        audioKeeper->init(audioDirector, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        audioKeeper->setIsForceInvalidSe(true);
    } else {
        audioKeeper->init(audioDirector, pSeName, pBgmName ? pBgmName : pSeName, getTransPtr(pActor), pActor->getBaseMtx(), pActor->mModelKeeper, nullptr);
    }

    pActor->initAudioKeeper(audioKeeper);
}

/**
 * Creates the audio keeper of an actor with a custom position and matrix.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pSeName The sound effect user name.
 * @param pBgmName The music user name, or nullptr to use the sound effect name.
 * @param pPos The sound position.
 * @param pMtx The sound matrix.
 */
void initActorAudioKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pSeName, const char* pBgmName, const sead::Vector3f* pPos, const sead::Matrix34f* pMtx) {
    AudioKeeper* audioKeeper = new AudioKeeper();
    if (rInfo.mAudioDirector->isForceInvalidSe()) {
        audioKeeper->init(rInfo.mAudioDirector, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        audioKeeper->setIsForceInvalidSe(true);
    } else {
        audioKeeper->init(rInfo.mAudioDirector, pSeName, pBgmName ? pBgmName : pSeName, pPos, pMtx, pActor->mModelKeeper, nullptr);
    }

    pActor->initAudioKeeper(audioKeeper);
}

/**
 * Creates the audio keeper of an actor without 3D sound.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pSeName The sound effect user name.
 * @param pBgmName The music user name, or nullptr to use the sound effect name.
 */
void initActorAudioKeeperWithout3D(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pSeName, const char* pBgmName) {
    AudioKeeper* audioKeeper = new AudioKeeper();
    if (rInfo.mAudioDirector->isForceInvalidSe()) {
        audioKeeper->init(rInfo.mAudioDirector, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        audioKeeper->setIsForceInvalidSe(true);
    } else {
        audioKeeper->init(rInfo.mAudioDirector, pSeName, pBgmName ? pBgmName : pSeName, nullptr, nullptr, nullptr, nullptr);
    }

    pActor->initAudioKeeper(audioKeeper);
}

/**
 * Creates the ocean wave keeper of an actor from a yaml.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rIter The ocean wave yaml.
 */
void initActorOceanWaveKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, ByamlIter& rIter) {
    OceanWaveKeeper* keeper = new OceanWaveKeeper(rInfo.mOceanWaveDirector);
    keeper->init(pActor->getName(), rIter);
    pActor->initOceanWaveKeeper(keeper);
}

/**
 * Creates the ocean wave keeper of an actor from an object resource yaml.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The archive name.
 * @param pFileName The yaml name.
 */
void initActorOceanWaveKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName, const char* pFileName) {
    const u8* byml = tryGetBymlFromObjectResource(pArchiveName, pFileName);
    if (!byml) {
        return;
    }

    ByamlIter iter(byml);
    OceanWaveKeeper* keeper = new OceanWaveKeeper(rInfo.mOceanWaveDirector);
    keeper->init(pActor->getName(), iter);
    pActor->initOceanWaveKeeper(keeper);
}

/**
 * Creates the hit reaction keeper of an actor from its model resource.
 * @param pActor The actor.
 * @param pName The file suffix.
 */
void initHitReactionKeeper(LiveActor* pActor, const char* pName) {
    HitReactionKeeper* keeper = HitReactionKeeper::tryCreate(pActor, getModelResource(pActor), pName);
    if (keeper) {
        pActor->mHitReactionKeeper = keeper;
    }
}

/**
 * Creates the hit reaction keeper of an actor from a resource.
 * @param pActor The actor.
 * @param pResource The resource.
 * @param pName The file suffix.
 */
void initHitReactionKeeper(LiveActor* pActor, const Resource* pResource, const char* pName) {
    HitReactionKeeper* keeper = HitReactionKeeper::tryCreate(pActor, pResource, pName);
    if (keeper) {
        pActor->mHitReactionKeeper = keeper;
    }
}

/**
 * Creates the shadow keeper of an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rIter The shadow yaml.
 * @param pUnused Unused.
 * @param rUnused1 Unused.
 * @param rUnused2 Unused.
 */
void initActorShadowKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const ByamlIter& rIter, const char* pUnused, const sead::SafeString& rUnused1, const sead::SafeString& rUnused2) {
    bool isIgnoreShadowMaskYaml = false;
    tryGetArg(&isIgnoreShadowMaskYaml, rInfo, "IsIgnoreShadowMaskYaml");
    ShadowKeeper* shadowKeeper = new ShadowKeeper(isIgnoreShadowMaskYaml);
    shadowKeeper->init(pActor, rInfo, rIter);
    pActor->initShadowKeeper(shadowKeeper);
}

/**
 * Creates the collision parts of an actor from its model resource.
 * @param pActor The actor.
 * @param rName The collision file name.
 * @param pSensor The sensor the collision belongs to.
 * @param pMtx The matrix to sync the collision with.
 */
void initActorCollision(LiveActor* pActor, const sead::SafeString& rName, HitSensor* pSensor, const sead::Matrix34f* pMtx) {
    initActorCollisionWithResource(pActor, getModelResource(pActor), rName, pSensor, pMtx, nullptr);
}

/**
 * Creates the collision parts of an actor from a resource.
 * @param pActor The actor.
 * @param pResource The resource.
 * @param rName The collision file name.
 * @param pSensor The sensor the collision belongs to.
 * @param pMtx The matrix to sync the collision with.
 * @param pSuffix The init file suffix, or nullptr.
 */
void initActorCollisionWithResource(LiveActor* pActor, Resource* pResource, const sead::SafeString& rName, HitSensor* pSensor, const sead::Matrix34f* pMtx, const char* pSuffix) {
    StringTmp<256> kclName("%s.kcl", rName.cstr());
    StringTmp<256> attributeName("%sAttribute.byml", rName.cstr());
    if (!pResource->isExistFile(kclName)) {
        return;
    }

    void* kcl = const_cast<void*>(pResource->getKcl(rName));
    const u8* attribute = pResource->isExistFile(attributeName) ? pResource->getByml(StringTmp<256>("%sAttribute", rName.cstr())) : nullptr;
    const char* specialPurpose = nullptr;
    s32 priority = -1;
    StringTmp<64> initName("InitCollision");
    if (pSuffix) {
        initName.format("InitCollision%s", pSuffix);
    }

    StringTmp<64> initFileName("%s.byml", initName.cstr());
    if (pResource->isExistFile(initFileName.cstr())) {
        ByamlIter iter(pResource->getByml(initName.cstr()));
        iter.tryGetStringByKey(&specialPurpose, "SpecialPurpose");
        iter.tryGetIntByKey(&priority, "Priority");
    }

    initActorCollisionWithFilePtr(pActor, kcl, attribute, pSensor, pMtx, specialPurpose, priority);
}

/**
 * Creates the collision parts of an actor from an archive.
 * @param pActor The actor.
 * @param rArchiveName The archive name.
 * @param rName The collision file name.
 * @param pSensor The sensor the collision belongs to.
 * @param pMtx The matrix to sync the collision with.
 */
void initActorCollisionWithArchiveName(LiveActor* pActor, const sead::SafeString& rArchiveName, const sead::SafeString& rName, HitSensor* pSensor, const sead::Matrix34f* pMtx) {
    initActorCollisionWithResource(pActor, findOrCreateResource(rArchiveName, nullptr), rName, pSensor, pMtx, nullptr);
}

/**
 * Creates the collision parts of an actor from file data.
 * @param pActor The actor.
 * @param pKcl The collision data.
 * @param pAttribute The attribute yaml.
 * @param pSensor The sensor the collision belongs to.
 * @param pMtx The matrix to sync the collision with.
 * @param pSpecialPurpose The special purpose name.
 * @param priority The collision priority.
 */
void initActorCollisionWithFilePtr(LiveActor* pActor, void* pKcl, const void* pAttribute, HitSensor* pSensor, const sead::Matrix34f* pMtx, const char* pSpecialPurpose, s32 priority) {
    CollisionParts* parts = new CollisionParts(pKcl, pAttribute);
    parts->_165 = true;
    parts->mSpecialPurpose = pSpecialPurpose;
    parts->mPriority = priority;
    sead::Matrix34f mtx;
    makeMtxSRT(&mtx, pActor);
    parts->mSensor = pSensor;
    parts->initParts(mtx);
    parts->mSyncCollisionMtx = pMtx;
    pActor->getCollisionDirector()->getActivePartsKeeper()->addCollisionParts(parts);
    parts->invalidateBySystem();
    pActor->mCollisionParts = parts;
}

/**
 * Creates the item keeper of an actor from an item list yaml.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The archive name.
 * @param pFileName The yaml name.
 */
void initActorItemKeeper(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName, const char* pFileName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    if (!resource) {
        return;
    }

    ByamlIter rootIter(resource->getByml(pFileName));
    ByamlIter initInfoIter;
    s32 addItemNum = 0;
    if (rootIter.tryGetIterByKey(&initInfoIter, "InitInfo")) {
        s32 num = 0;
        if (initInfoIter.tryGetIntByKey(&num, "AddItemNum")) {
            addItemNum = num;
        }
    }

    ByamlIter itemListIter;
    ByamlIter listIter(rootIter.tryGetIterByKey(&itemListIter, "ItemList") ? itemListIter : rootIter);
    s32 itemNum = listIter.getSize() + addItemNum;
    if (itemNum <= 0) {
        return;
    }

    pActor->initItemKeeper(itemNum);
    for (s32 i = 0; i < itemNum; i++) {
        ByamlIter itemIter;
        if (!listIter.tryGetIterByIndex(&itemIter, i)) {
            continue;
        }

        const char* itemName = nullptr;
        if (!itemIter.tryGetStringByKey(&itemName, "Item")) {
            continue;
        }

        const char* timing = nullptr;
        itemIter.tryGetStringByKey(&timing, "Timing");
        const char* factor = nullptr;
        itemIter.tryGetStringByKey(&factor, "Factor");
        bool isNoDeclare = tryGetByamlKeyBoolOrFalse(itemIter, "IsNoDeclarePlacementActor");
        addItem(pActor, rInfo, itemName, timing, factor, isNoDeclare);
    }
}

/**
 * Creates the alpha control of an actor from its init file.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The archive name.
 * @param pFileName The init file name, or nullptr for the default.
 */
void initActorAlphaCtrl(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName, const char* pFileName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    if (!resource) {
        return;
    }

    const char* fileName = pFileName ? pFileName : "InitAlphaCtrl";
    ByamlIter iter;
    if (!tryGetActorInitFileIter(&iter, resource, fileName, nullptr)) {
        return;
    }

    pActor->initActorAlphaCtrl(new ActorAlphaCtrl(iter, pActor), rInfo);
}

/**
 * Creates the pre-pass light keeper of an actor.
 * @param pActor The actor.
 * @param pResource Unused.
 * @param rInfo The actor init info.
 * @param rIter The light yaml.
 * @param pUnused Unused.
 */
void initActorPrePassLightKeeper(LiveActor* pActor, const Resource* pResource, const ActorInitInfo& rInfo, const ByamlIter& rIter, const char* pUnused) {
    bool isIgnorePrePassLightYaml = false;
    tryGetArg(&isIgnorePrePassLightYaml, rInfo, "IsIgnorePrePassLightYaml");
    ActorPrePassLightKeeper* lightKeeper = new ActorPrePassLightKeeper(isIgnorePrePassLightYaml);
    lightKeeper->init(pActor, rInfo, rIter);
    pActor->initActorPrePassLightKeeper(lightKeeper);
}

/**
 * Creates a sub actor keeper without an init file.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param maxSubActors The maximum number of sub actors.
 */
void initSubActorKeeperNoFile(LiveActor* pActor, const ActorInitInfo& rInfo, s32 maxSubActors) {
    pActor->initSubActorKeeper(SubActorKeeper::createNoFile(pActor, rInfo, maxSubActors));
}

/**
 * Registers a sub actor that syncs its clipping with the actor.
 * @param pActor The actor.
 * @param pSubActor The sub actor.
 * @param isSyncHide Whether the sub actor also syncs appearance and hiding.
 */
void registerSubActorSyncClipping(LiveActor* pActor, LiveActor* pSubActor, bool isSyncHide) {
    pActor->mSubActorKeeper->registerSubActor(pSubActor, isSyncHide ? 7 : 2);
}

/**
 * Stops all sub actors from syncing their clipping.
 * @param pActor The actor.
 */
void setSubActorOffSyncClipping(LiveActor* pActor) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    for (s32 i = 0; i < keeper->mCount; i++) {
        keeper->mInfos[i]->mSyncType &= ~2;
    }
}

/**
 * Makes all sub actors sync their appearance.
 * @param pActor The actor.
 */
void setSubActorOnSyncAppear(LiveActor* pActor) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    for (s32 i = 0; i < keeper->mCount; i++) {
        keeper->mInfos[i]->mSyncType |= 1;
    }
}
}  // namespace al
