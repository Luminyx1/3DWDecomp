#include "Library/LiveActor/Util/ActorInitUtil.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Actor/ActorPoseKeeper.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/Factory/ActorFactory.hpp"
#include "Library/Item/ActorScoreKeeper.hpp"
#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/HitSensor/SensorFunction.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/ActorParamHolder.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Nerve/NerveActionCtrl.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Screen/ScreenPointerUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Action/Common/ActorActionKeeper.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"
#include "Project/Play/Actor/ActorAlphaCtrl.hpp"

namespace al {
namespace {
using PoseKeeperCreatorFunction = void (*)(LiveActor*);

const NameToCreator<PoseKeeperCreatorFunction> sActorPoseTable[] = {
    {"TRSV", initActorPoseTRSV}, {"TRMSV", initActorPoseTRMSV}, {"TFSV", initActorPoseTFSV},
    {"TFGSV", initActorPoseTFGSV}, {"TQSV", initActorPoseTQSV},
};

void initActorModel(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                    const char* pSuffix, const sead::SafeString& rArchiveName) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitModel", pSuffix)) {
        return;
    }

    const char* animArc = nullptr;
    iter.tryGetStringByKey(&animArc, "AnimArc");
    const char* textureArc = nullptr;
    iter.tryGetStringByKey(&textureArc, "TextureArc");
    s32 blendAnimMax = 1;
    iter.tryGetIntByKey(&blendAnimMax, "BlendAnimMax");
    initActorModelKeeperWithInitFile(
        pActor, rInfo, iter, rArchiveName.cstr(), blendAnimMax,
        (animArc != nullptr) ? StringTmp<256>("ObjectData/%s", animArc).cstr() : nullptr,
        (textureArc != nullptr) ? StringTmp<256>("ObjectData/%s", textureArc).cstr() : nullptr);

    s32 partialAnimSlotNum = 0;
    iter.tryGetIntByKey(&partialAnimSlotNum, "PartialAnimSlotNum");
    s32 partialAnimGroupNum = partialAnimSlotNum;
    iter.tryGetIntByKey(&partialAnimGroupNum, "PartialAnimGroupNum");
    s32 partialAnimPartsListBufferSize = 16;
    iter.tryGetIntByKey(&partialAnimPartsListBufferSize, "PartialAnimPartsListBufferSize");

    if (partialAnimGroupNum >= 1) {
        initPartialSklAnim(pActor, partialAnimSlotNum, partialAnimGroupNum,
                           partialAnimPartsListBufferSize);
    }

    bool isFixedModel = false;
    iter.tryGetBoolByKey(&isFixedModel, "IsFixedModel");

    if (isFixedModel) {
        setFixedModelFlag(pActor);
    }

    bool isIgnoreUpdateOnDrawClipping = false;
    iter.tryGetBoolByKey(&isIgnoreUpdateOnDrawClipping, "IsIgnoreUpdateOnDrawClipping");

    if (isIgnoreUpdateOnDrawClipping) {
        setIgnoreUpdateDrawClipping(pActor, true);
    }

    s32 wetType = 0;
    tryGetArg(&wetType, rInfo, "WetType");

    switch (wetType) {
    case 1:
        changeEnvTextureWetObj(pActor);
        break;
    case 2:
        changeEnvTextureWetObjStrong(pActor);
        break;
    default:
        break;
    }
}

void initActorLod(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                  const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitLod", pSuffix)) {
        return;
    }

    f32 switchDistances[4] = {15000.0f, 30000.0f, 3.4028235e38f, 3.4028235e38f};
    s32 lodNum = 0;

    for (s32 i = 0; i < 4; i++) {
        sead::FixedSafeString<32> key;
        key.format("Lod%dSwitchDist", i + 1);

        if (iter.tryGetFloatByKey(&switchDistances[i], key.cstr())) {
            lodNum = i + 1;
        }
    }

    const char* lodSettingName =
        static_cast<GraphicsSystemInfo*>(rInfo.mActorSceneInfo._78)->mLodSettingName;
    ByamlIter lodSettingIter;

    if (iter.tryGetIterByKey(&lodSettingIter, lodSettingName)) {
        for (s32 i = 0; i < 4; i++) {
            sead::FixedSafeString<32> key;
            key.format("Lod%dSwitchDist", i + 1);

            if (iter.tryGetFloatByKey(&switchDistances[i], key.cstr())) {
                lodNum = sead::Mathi::max(lodNum, i + 1);
            }
        }
    }

    bool isDisableDemoLod = false;
    iter.tryGetBoolByKey(&isDisableDemoLod, "DisableDemoLod");
    setModelLodParams(pActor, switchDistances, lodNum, isDisableDemoLod);
}

void initActorExecutor(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                       const char* pSuffix) {
    bool isUsingDepthShadow = false;
    tryGetArg(&isUsingDepthShadow, rInfo, "UsingDepthShadow");

    if (isUsingDepthShadow && pActor->mModelKeeper != nullptr) {
        registerExecutorActorDraw(pActor, rInfo.mExecuteDirector, "デプスシャドウ[キャラクター]");
    }

    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitExecutor", pSuffix)) {
        return;
    }

    const char* updateCategoryName = nullptr;
    const char* drawCategoryName = nullptr;
    ByamlIter listIter;

    if (iter.tryGetIterByKey(&listIter, "Updater")) {
        if (listIter.isTypeArray()) {
            ByamlIter entryIter;

            for (s32 i = 0; listIter.tryGetIterByIndex(&entryIter, i); i++) {
                entryIter.tryGetStringByKey(&updateCategoryName, "CategoryName");
                initExecutorUpdate(pActor, rInfo, updateCategoryName);
            }
        } else {
            listIter.tryGetStringByKey(&updateCategoryName, "CategoryName");
            initExecutorUpdate(pActor, rInfo, updateCategoryName);
        }
    }

    if (iter.tryGetIterByKey(&listIter, "Drawer")) {
        if (listIter.isTypeArray()) {
            ByamlIter entryIter;

            for (s32 i = 0; listIter.tryGetIterByIndex(&entryIter, i); i++) {
                entryIter.tryGetStringByKey(&drawCategoryName, "CategoryName");
                initExecutorDraw(pActor, rInfo, drawCategoryName);
            }
        } else {
            listIter.tryGetStringByKey(&drawCategoryName, "CategoryName");
            initExecutorDraw(pActor, rInfo, drawCategoryName);
        }
    }
}

bool initActorPoseKeeper(const char* pPose, LiveActor* pActor) {
    s32 poseIndex = -1;

    for (s32 i = 0; i < 5; i++) {
        if (isEqualString(sActorPoseTable[i].name, pPose)) {
            poseIndex = i;
            break;
        }
    }

    if (poseIndex == -1) {
        return false;
    }

    sActorPoseTable[poseIndex].func(pActor);
    return true;
}

void initActorPose(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                   const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitPose", pSuffix)) {
        return;
    }

    const char* pose = nullptr;

    if (!iter.tryGetStringByKey(&pose, "Pose") || !pose) {
        return;
    }

    initActorPoseKeeper(pose, pActor);
}

void initActorScale(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                    const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitScale", pSuffix)) {
        return;
    }

    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};

    if (!tryGetByamlScale(&scale, iter, "Scale")) {
        return;
    }

    const sead::Vector3f& actorScale = getScale(pActor);
    scale.x = scale.x * actorScale.x;
    scale.y = scale.y * actorScale.y;
    scale.z = scale.z * actorScale.z;
    setScale(pActor, scale);
}

void initActorPrePassLight(LiveActor* pActor, const ActorInitInfo& rInfo,
                           const Resource* pResource, const char* pSuffix) {
    ByamlIter iter;
    StringTmp<64> fileName;

    if (!tryGetActorInitFileIterAndName(&iter, &fileName, pResource, "InitPrePassLight",
                                        pSuffix)) {
        return;
    }

    initActorPrePassLightKeeper(pActor, pResource, rInfo, iter, fileName.cstr());
}

void initActorSensor(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                     const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitSensor", pSuffix)) {
        return;
    }

    s32 sensorNum = iter.getSize();

    if (sensorNum <= 0) {
        return;
    }

    pActor->initHitSensor(sensorNum);

    for (s32 i = 0; i < sensorNum; i++) {
        ByamlIter sensorIter;

        if (!iter.tryGetIterByIndex(&sensorIter, i)) {
            continue;
        }

        const char* name = nullptr;

        if (!sensorIter.tryGetStringByKey(&name, "Name")) {
            continue;
        }

        const char* typeName = nullptr;

        if (!sensorIter.tryGetStringByKey(&typeName, "Type")) {
            continue;
        }

        f32 radius = 0.0f;
        sensorIter.tryGetFloatByKey(&radius, "Radius");
        s32 maxCount = 8;
        sensorIter.tryGetIntByKey(&maxCount, "MaxCount");
        sead::Vector3f offset = sead::Vector3f::zero;
        tryGetByamlV3f(&offset, sensorIter);
        HitSensorType type = alSensorFunction::findSensorTypeByName(typeName);

        if (type == HitSensorType::CollisionParts) {
            maxCount = 0;
        }

        addHitSensor(pActor, rInfo, name, static_cast<u32>(type), radius, maxCount, offset);
        const char* jointName = nullptr;
        sensorIter.tryGetStringByKey(&jointName, "Joint");

        if (jointName != nullptr) {
            setHitSensorJointMtx(pActor, name, jointName);
        }
    }
}

void initActorCollision(LiveActor* pActor, const ActorInitInfo& rInfo, Resource* pResource,
                        const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitCollision", pSuffix)) {
        return;
    }

    const char* name = nullptr;
    iter.tryGetStringByKey(&name, "Name");
    StringTmp<256> collisionName;

    if (name == nullptr) {
        name = getBaseName(pResource->getArchiveName());
    }

    const char* sensorName = nullptr;
    HitSensor* sensor = nullptr;

    if (iter.tryGetStringByKey(&sensorName, "Sensor")) {
        sensor = getHitSensor(pActor, sensorName);
    }

    const char* jointName = nullptr;
    iter.tryGetStringByKey(&jointName, "Joint");
    const sead::Matrix34f* jointMtx = nullptr;

    if (jointName != nullptr) {
        jointMtx = getJointMtxPtr(pActor, jointName);
    }

    initActorCollisionWithResource(pActor, pResource, name, sensor, jointMtx, pSuffix);
}

void initActorCollider(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                       const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitCollider", pSuffix)) {
        return;
    }

    f32 radius = 0.0f;
    iter.tryGetFloatByKey(&radius, "Radius");
    sead::Vector3f offset = sead::Vector3f::zero;
    tryGetByamlV3f(&offset, iter);
    pActor->initCollider(radius, offset.y, 0);
}

void initActorEffect(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                     const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitEffect", pSuffix)) {
        return;
    }

    const char* name = nullptr;

    if (!iter.tryGetStringByKey(&name, "Name")) {
        return;
    }

    initActorEffectKeeper(pActor, rInfo, name, false);
}

void initActorSound(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                    const char* pSuffix) {
    ByamlIter iter;

    if (tryGetActorInitFileIter(&iter, pResource, "InitSound", pSuffix)) {
        const char* name = nullptr;
        iter.tryGetStringByKey(&name, "Name");
        initActorAudioKeeper(pActor, rInfo, name, name);
    } else if (tryGetActorInitFileIter(&iter, pResource, "InitAudio", pSuffix)) {
        const char* seUserName = nullptr;
        const char* bgmUserName = nullptr;
        iter.tryGetStringByKey(&seUserName, "SeUserName");
        iter.tryGetStringByKey(&bgmUserName, "BgmUserName");
        initActorAudioKeeper(pActor, rInfo, seUserName, bgmUserName);
    }
}

void initActorOceanWave(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                        const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitOceanWave", pSuffix)) {
        return;
    }

    initActorOceanWaveKeeper(pActor, rInfo, iter);
}

void initActorRail(LiveActor* pActor, const ActorInitInfo& rInfo) {
    if (isExistRail(rInfo)) {
        pActor->initRailKeeper(rInfo);
    }
}

void initActorGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo,
                            const ByamlIter& rClippingIter) {
    if (rClippingIter.isExistKey("NoGroupClipping")) {
        return;
    }

    ByamlIter groupIter;

    if (!rClippingIter.tryGetIterByKey(&groupIter, "GroupClipping")) {
        return;
    }

    s32 maxCount = 16;
    groupIter.tryGetIntByKey(&maxCount, "MaxCount");
    initGroupClipping(pActor, rInfo, maxCount);
}

void initActorClippingFile(LiveActor* pActor, const ActorInitInfo& rInfo,
                           const Resource* pResource, const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitClipping", pSuffix)) {
        return;
    }

    bool isInvalidate = false;
    iter.tryGetBoolByKey(&isInvalidate, "Invalidate");

    if (isInvalidate) {
        invalidateClipping(pActor);
    }

    bool isNoCollisionClipping = false;
    iter.tryGetBoolByKey(&isNoCollisionClipping, "NoCollisionClipping");

    if (isNoCollisionClipping) {
        setNoCollisionClip(pActor, true);
    }

    f32 radius = 0.0f;

    if (iter.tryGetFloatByKey(&radius, "Radius")) {
        setClippingInfo(pActor, radius, nullptr);
    } else if (pActor->mModelKeeper != nullptr) {
        const sead::Vector3f& scale = getScale(pActor);
        f32 maxXY = sead::Mathf::max(sead::Mathf::abs(scale.x), sead::Mathf::abs(scale.y));
        f32 maxXYZ = sead::Mathf::max(maxXY, sead::Mathf::abs(scale.z));
        setClippingInfo(pActor, calcModelBoundingSphereRadius(pActor) * maxXYZ, nullptr);
    }

    f32 nearDistance = 0.0f;
    f32 farDistance = 0.0f;

    if (iter.tryGetFloatByKey(&nearDistance, "NearClipDistance")) {
        setClippingNearDistance(pActor, nearDistance);
    }

    if (iter.tryGetFloatByKey(&farDistance, "FarAreaDistance") &&
        iter.tryGetFloatByKey(&nearDistance, "NearAreaDistance")) {
        setClippingNearFarDistance(pActor, nearDistance, farDistance);
    }

    sead::Vector3f offset = sead::Vector3f::zero;

    if (tryGetByamlV3f(&offset, iter, "Offset")) {
        setClippingOffset(pActor, offset);
    }

    f32 shadowDisappearDistance = 0.0f;

    if (iter.tryGetFloatByKey(&shadowDisappearDistance, "ShadowDisappearDistance") &&
        shadowDisappearDistance > 0.0f) {
        setShadowClippingDistance(pActor, shadowDisappearDistance);
    }

    f32 drawClippingRadius = 0.0f;

    if (iter.tryGetFloatByKey(&drawClippingRadius, "DrawClippingRadius")) {
        setDrawClippingRadius(pActor, drawClippingRadius);
    }

    initActorGroupClipping(pActor, rInfo, iter);
}

void initActorShadowMask(LiveActor* pActor, const ActorInitInfo& rInfo,
                         const Resource* pResource, const char* pSuffix,
                         const sead::SafeString& rFileName, const sead::SafeString& rArchiveName) {
    bool isUsingDepthShadow = false;
    tryGetArg(&isUsingDepthShadow, rInfo, "UsingDepthShadow");
    ByamlIter iter;
    StringTmp<128> fileName;

    if (!tryGetActorInitFileIterAndName(&iter, &fileName, pResource, "InitShadowMask", pSuffix)) {
        return;
    }

    if (iter.isExistKey("IgnoreShadowMaskYaml")) {
        return;
    }

    initActorShadowKeeper(pActor, rInfo, iter, fileName.cstr(), rFileName, rArchiveName);

    if (isUsingDepthShadow) {
        invalidateShadowIntensityAll(pActor);
    }
}

void initActorFlag(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                   const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitFlag", pSuffix)) {
        return;
    }

    ByamlIter materialCodeIter;

    if (iter.tryGetIterByKey(&materialCodeIter, "MaterialCode")) {
        validateMaterialCode(pActor);
    }
}

void initActorItem(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                   const char* pSuffix, const sead::SafeString& rArchiveName) {
    StringTmp<64> fileName;

    if (!tryGetActorInitFileIterAndName(nullptr, &fileName, pResource, "InitItem", pSuffix)) {
        return;
    }

    initActorItemKeeper(pActor, rInfo, rArchiveName.cstr(), fileName.cstr());
}

void initActorScore(LiveActor* pActor, const ActorInitInfo& rInfo, const Resource* pResource,
                    const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitScore", pSuffix)) {
        return;
    }

    pActor->initScoreKeeper();
    pActor->mScoreKeeper->init(iter);
}

void initActorScreenPoint(LiveActor* pActor, const ActorInitInfo& rInfo,
                          const Resource* pResource, const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetActorInitFileIter(&iter, pResource, "InitScreenPoint", pSuffix)) {
        return;
    }

    s32 targetNum = iter.getSize();

    if (targetNum <= 0) {
        return;
    }

    pActor->initScreenPointKeeper(targetNum);

    for (s32 i = 0; i < targetNum; i++) {
        ByamlIter targetIter;

        if (!iter.tryGetIterByIndex(&targetIter, i)) {
            continue;
        }

        const char* name = nullptr;

        if (!targetIter.tryGetStringByKey(&name, "Name")) {
            continue;
        }

        f32 radius = 0.0f;
        targetIter.tryGetFloatByKey(&radius, "Radius");
        sead::Vector3f offset = sead::Vector3f::zero;
        tryGetByamlV3f(&offset, targetIter);
        const char* jointName = nullptr;
        targetIter.tryGetStringByKey(&jointName, "Joint");
        addScreenPointTarget(pActor, rInfo, name, radius, jointName, offset);
    }
}

void initActorAlphaCtrlFile(LiveActor* pActor, const ActorInitInfo& rInfo,
                            const Resource* pResource, const char* pSuffix) {
    if (!rInfo.mActorSceneInfo.isSingleMode) {
        return;
    }

    pActor->initActorAlphaCtrl(ActorAlphaCtrl::tryCreate(pActor, pResource, pSuffix), rInfo);
}

void initActorParamHolder(LiveActor* pActor) {
    ActorParamHolder* paramHolder = ActorParamHolder::tryCreate(pActor);

    if (paramHolder != nullptr) {
        pActor->mActorParamHolder = paramHolder;
    }
}

void initActorAction(LiveActor* pActor, const sead::SafeString& rFileName, const char* pSuffix) {
    const char* actionName = getBaseName(rFileName.cstr());
    pActor->initActionKeeper(actionName, pSuffix);

    if (pActor->mModelKeeper == nullptr) {
        return;
    }

    if (!tryStartAction(pActor, actionName) && pActor->mActionKeeper != nullptr) {
        pActor->mActionKeeper->startAction(actionName);
    }
}

void initFarLodActor(LiveActor* pActor, const ActorInitInfo& rInfo) {
    if (calcLinkChildNum(rInfo, "FarLOD") < 1) {
        return;
    }

    ActorInitInfo farLodInfo;
    PlacementInfo farLodPlacementInfo;
    getLinksInfoByIndex(&farLodPlacementInfo, *rInfo.mPlacementInfo, "FarLOD", 0);
    farLodInfo.initViewIdSelf(&farLodPlacementInfo, rInfo);
    const char* displayName = nullptr;
    getDisplayName(&displayName, farLodPlacementInfo);
    LiveActor* farLodActor = new LiveActor(displayName);
    farLodActor->setIsFarLodModel(true);
    StringTmp<256> modelName;
    StringTmp<256> farLodArchiveName;
    makeMapPartsModelName(&modelName, &farLodArchiveName, *farLodInfo.mPlacementInfo);
    Resource* farLodResource = findOrCreateResource(farLodArchiveName, nullptr);

    if (farLodActor->getSceneInfo() == nullptr) {
        initActorSceneInfo(farLodActor, rInfo);
    }

    farLodActor->initPoseKeeper(pActor->mActorPoseKeeper);
    initActorModel(farLodActor, rInfo, farLodResource, nullptr, farLodArchiveName);
    initActorLod(farLodActor, rInfo, farLodResource, nullptr);
    initActorExecutor(farLodActor, rInfo, farLodResource, nullptr);
    farLodActor->init(farLodInfo);
    pActor->setFarLodActor(farLodActor);
    farLodActor->makeActorAppeared();
    farLodActor->startClipped();

    if (farLodActor->mModelKeeper != nullptr && farLodActor->mModelKeeper->getLodNum() >= 1) {
        pActor->_142 = true;
    }
}

LiveActor* createActorFromFactory(const ActorFactory& rFactory, const ActorInitInfo& rInfo) {
    const char* objectName = nullptr;
    getObjectName(&objectName, rInfo);
    const char* className = nullptr;
    getClassName(&className, rInfo);
    CreationFuncPtr creator = nullptr;
    rFactory.getEntryIndex(&creator, className);

    if (!creator) {
        return nullptr;
    }

    const char* displayName;
    getDisplayName(&displayName, rInfo);
    LiveActor* actor = creator(displayName);
    actor->init(rInfo);
    return actor;
}

void makeMapPartsModelAndFolderName(sead::BufferedSafeString* pModelName,
                                    sead::BufferedSafeString* pFolderName,
                                    const PlacementInfo& rInfo) {
    const char* name = nullptr;

    if (alPlacementFunction::tryGetModelName(&name, rInfo) && !isEqualString(name, "")) {
        pModelName->copy(name);
        pFolderName->copy("ObjectData");
    } else {
        tryGetStringArg(&name, rInfo, "UnitConfigName");
        pModelName->copy(name);
        pFolderName->copy("ObjectData");
    }
}

const char* getChangeModelName(const ActorInitInfo& rInfo) {
    const char* name = nullptr;

    if (alPlacementFunction::tryGetModelName(&name, rInfo)) {
        return name;
    }

    if (tryGetObjectName(&name, rInfo)) {
        return name;
    }

    return nullptr;
}

void initActorImpl(LiveActor* pActor, const ActorInitInfo& rInfo,
                   const sead::SafeString& rFolderName, const sead::SafeString& rFileName,
                   const char* pSuffix, s32 maxSubActors) {
    StringTmp<256> archiveName("%s/%s", rFolderName.cstr(), rFileName.cstr());
    Resource* resource = findOrCreateResource(archiveName, nullptr);

    if (pActor->getSceneInfo() == nullptr) {
        initActorSceneInfo(pActor, rInfo);
    }

    initActorPose(pActor, rInfo, resource, pSuffix);
    initActorSRT(pActor, rInfo);
    initActorScale(pActor, rInfo, resource, pSuffix);
    initActorModel(pActor, rInfo, resource, pSuffix, archiveName);
    initActorLod(pActor, rInfo, resource, pSuffix);
    initActorPrePassLight(pActor, rInfo, resource, pSuffix);
    initActorExecutor(pActor, rInfo, resource, pSuffix);
    initActorSensor(pActor, rInfo, resource, pSuffix);
    initActorCollision(pActor, rInfo, resource, pSuffix);
    initActorCollider(pActor, rInfo, resource, pSuffix);
    initActorEffect(pActor, rInfo, resource, pSuffix);
    initActorSound(pActor, rInfo, resource, pSuffix);
    initActorOceanWave(pActor, rInfo, resource, pSuffix);
    initActorRail(pActor, rInfo);
    initStageSwitch(pActor, rInfo);
    initActorClipping(pActor, rInfo);
    initActorClippingFile(pActor, rInfo, resource, pSuffix);
    initActorShadowMask(pActor, rInfo, resource, pSuffix, rFileName, archiveName);
    initActorFlag(pActor, rInfo, resource, pSuffix);
    initActorItem(pActor, rInfo, resource, pSuffix, archiveName);
    initActorScore(pActor, rInfo, resource, pSuffix);
    initActorScreenPoint(pActor, rInfo, resource, pSuffix);
    initHitReactionKeeper(pActor, resource, pSuffix);
    initActorAlphaCtrlFile(pActor, rInfo, resource, pSuffix);
    initActorParamHolder(pActor);
    initActorAction(pActor, rFileName, pSuffix);

    if (pActor->getNerveKeeper() != nullptr && pActor->getNerveKeeper()->mActionCtrl != nullptr) {
        resetNerveActionForInit(pActor);
    }

    if (pActor->mSubActorKeeper == nullptr) {
        SubActorKeeper* subActorKeeper =
            SubActorKeeper::tryCreate(pActor, rInfo, pSuffix, maxSubActors);
        if (subActorKeeper != nullptr) {
            pActor->initSubActorKeeper(subActorKeeper);
        }
    }

    initFarLodActor(pActor, rInfo);
}
}  // namespace

/**
 * Initializes an actor from its object data archive.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActor(LiveActor* pActor, const ActorInitInfo& rInfo) {
    const char* objectName = nullptr;
    tryGetObjectName(&objectName, rInfo);
    initActorImpl(pActor, rInfo, "ObjectData", objectName, nullptr, 0);
}

/**
 * Initializes an actor from its object data archive with a file suffix.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pSuffix The init file suffix.
 */
void initActorSuffix(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pSuffix) {
    const char* objectName = nullptr;
    tryGetObjectName(&objectName, rInfo);
    initActorImpl(pActor, rInfo, "ObjectData", objectName, pSuffix, 0);
}

/**
 * Initializes an actor from the archive of its placed model.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActorChangeModel(LiveActor* pActor, const ActorInitInfo& rInfo) {
    const char* modelName = getChangeModelName(rInfo);
    initActorImpl(pActor, rInfo, "ObjectData", modelName, nullptr, 0);
}

/**
 * Initializes an actor from the archive of its placed model with a file suffix.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pSuffix The init file suffix.
 */
void initActorChangeModelSuffix(LiveActor* pActor, const ActorInitInfo& rInfo,
                                const char* pSuffix) {
    const char* modelName = getChangeModelName(rInfo);
    initActorImpl(pActor, rInfo, "ObjectData", modelName, pSuffix, 0);
}

/**
 * Initializes an actor from an object data archive.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rArchiveName The archive name.
 * @param pSuffix The init file suffix.
 */
void initActorWithArchiveName(LiveActor* pActor, const ActorInitInfo& rInfo,
                              const sead::SafeString& rArchiveName, const char* pSuffix) {
    initActorImpl(pActor, rInfo, "ObjectData", rArchiveName.cstr(), pSuffix, 0);
}

/**
 * Initializes an actor from an archive in a category folder.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rCategoryName The folder name.
 * @param rArchiveName The archive name.
 * @param pSuffix The init file suffix.
 */
void initActorWithArchiveCategoryName(LiveActor* pActor, const ActorInitInfo& rInfo,
                                      const sead::SafeString& rCategoryName,
                                      const sead::SafeString& rArchiveName,
                                      const char* pSuffix) {
    initActorImpl(pActor, rInfo, rCategoryName.cstr(), rArchiveName.cstr(), pSuffix, 0);
}

/**
 * Initializes an actor from an object data archive using its placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rArchiveName The archive name.
 * @param pSuffix The init file suffix.
 */
void initActorWithArchiveNameWithPlacementInfo(LiveActor* pActor, const ActorInitInfo& rInfo,
                                               const sead::SafeString& rArchiveName,
                                               const char* pSuffix) {
    initActorImpl(pActor, rInfo, "ObjectData", rArchiveName.cstr(), pSuffix, 0);
}

/**
 * Initializes an actor from an object data archive without a placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rArchiveName The archive name.
 * @param pSuffix The init file suffix.
 */
void initActorWithArchiveNameNoPlacementInfo(LiveActor* pActor, const ActorInitInfo& rInfo,
                                             const sead::SafeString& rArchiveName,
                                             const char* pSuffix) {
    PlacementInfo placementInfo;
    ActorInitInfo info;
    info.initViewIdHost(&placementInfo, rInfo);
    initActorImpl(pActor, info, "ObjectData", rArchiveName.cstr(), pSuffix, 0);
}

/**
 * Makes the model name and archive path of a map parts placement.
 * @param pModelName The model name.
 * @param pPath The archive path.
 * @param rInfo The placement info.
 */
void makeMapPartsModelName(sead::BufferedSafeString* pModelName, sead::BufferedSafeString* pPath,
                           const PlacementInfo& rInfo) {
    const char* modelName = nullptr;

    if (alPlacementFunction::tryGetModelName(&modelName, rInfo) &&
        !isEqualString(modelName, "")) {
        pModelName->copy(modelName);
        pPath->format("ObjectData/%s", modelName);
    } else {
        tryGetStringArg(&modelName, rInfo, "UnitConfigName");
        pModelName->copy(modelName);
        pPath->format("ObjectData/%s", modelName);
    }
}

/**
 * Makes the model name and archive path of a map parts actor.
 * @param pModelName The model name.
 * @param pPath The archive path.
 * @param rInfo The actor init info.
 */
void makeMapPartsModelName(sead::BufferedSafeString* pModelName, sead::BufferedSafeString* pPath,
                           const ActorInitInfo& rInfo) {
    makeMapPartsModelName(pModelName, pPath, *rInfo.mPlacementInfo);
}

/**
 * Checks whether a map parts init file exists for a suffix.
 * @param rInfo The actor init info.
 * @param pSuffix The suffix.
 * @return The suffix, or nullptr if no such file exists.
 */
const char* tryGetMapPartsSuffix(const ActorInitInfo& rInfo, const char* pSuffix) {
    StringTmp<64> fileName("InitActor%s", pSuffix);

    if (!tryGetMapPartsResourceYaml(rInfo, fileName.cstr())) {
        return nullptr;
    }

    return pSuffix;
}

/**
 * Initializes a map parts actor from the archive of its placed model.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pSuffix The init file suffix.
 * @param maxSubActors The number of sub actors to reserve.
 */
void initMapPartsActor(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pSuffix,
                       s32 maxSubActors) {
    StringTmp<256> modelName;
    StringTmp<256> folderName;
    makeMapPartsModelAndFolderName(&modelName, &folderName, *rInfo.mPlacementInfo);
    initActorImpl(pActor, rInfo, folderName, modelName, pSuffix, maxSubActors);
}

/**
 * Makes the init info of a linked actor.
 * @param pInfo The linked actor init info.
 * @param pPlacementInfo The linked placement info.
 * @param rInfo The actor init info.
 * @param pLinkName The link name.
 * @param index The link index.
 */
void getLinksActorInfo(ActorInitInfo* pInfo, PlacementInfo* pPlacementInfo,
                       const ActorInitInfo& rInfo, const char* pLinkName, s32 index) {
    getLinksInfoByIndex(pPlacementInfo, *rInfo.mPlacementInfo, pLinkName, index);
    pInfo->initViewIdSelf(pPlacementInfo, rInfo);
}

/**
 * Initializes an actor with the placement of a link.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pLinkName The link name.
 * @param index The link index.
 */
void initLinksActor(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pLinkName,
                    s32 index) {
    ActorInitInfo info;
    PlacementInfo placementInfo;
    getLinksInfoByIndex(&placementInfo, *rInfo.mPlacementInfo, pLinkName, index);
    info.initViewIdSelf(&placementInfo, rInfo);
    pActor->init(info);
}

/**
 * Creates the init info of a linked player restart position.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @return The init info.
 */
ActorInitInfo* createLinksPlayerActorInfo(LiveActor* pActor, const ActorInitInfo& rInfo) {
    s32 restartPosNum = calcLinkChildNum(rInfo, "PlayerRestartPos");
    PlacementInfo* placementInfo = new PlacementInfo();
    ActorInitInfo* info = new ActorInitInfo();

    if (restartPosNum == 1) {
        getLinksInfoByIndex(placementInfo, *rInfo.mPlacementInfo, "PlayerRestartPos", 0);
        info->initViewIdSelf(placementInfo, rInfo);
    }

    return info;
}

/**
 * Gets the class name of a linked actor.
 * @param rInfo The actor init info.
 * @param pLinkName The link name.
 * @param index The link index.
 * @return The class name.
 */
const char* getLinksActorClassName(const ActorInitInfo& rInfo, const char* pLinkName, s32 index) {
    ActorInitInfo info;
    PlacementInfo placementInfo;
    const char* className = nullptr;
    getLinksInfoByIndex(&placementInfo, *rInfo.mPlacementInfo, pLinkName, index);
    info.initViewIdSelf(&placementInfo, rInfo);
    getClassName(&className, info);
    return className;
}

/**
 * Gets the display name of a linked actor.
 * @param rInfo The actor init info.
 * @param pLinkName The link name.
 * @param index The link index.
 * @return The display name.
 */
const char* getLinksActorDisplayName(const ActorInitInfo& rInfo, const char* pLinkName,
                                     s32 index) {
    PlacementInfo placementInfo;
    getLinksInfoByIndex(&placementInfo, *rInfo.mPlacementInfo, pLinkName, index);
    const char* displayName = nullptr;
    getDisplayName(&displayName, placementInfo);
    return displayName;
}

/**
 * Initializes a created actor with the init info.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initCreateActorWithPlacementInfo(LiveActor* pActor, const ActorInitInfo& rInfo) {
    pActor->init(rInfo);
}

/**
 * Initializes a created actor with a placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param rPlacementInfo The placement info.
 */
void initCreateActorWithPlacementInfo(LiveActor* pActor, const ActorInitInfo& rInfo,
                                      const PlacementInfo& rPlacementInfo) {
    ActorInitInfo info;
    info.initViewIdSelf(&rPlacementInfo, rInfo);
    pActor->init(info);
}

/**
 * Initializes a created actor without a placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initCreateActorNoPlacementInfo(LiveActor* pActor, const ActorInitInfo& rInfo) {
    PlacementInfo placementInfo;
    ActorInitInfo info;
    info.initViewIdHost(&placementInfo, rInfo);
    pActor->init(info);
}

/**
 * Initializes a created actor without a placement or view id.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initCreateActorNoPlacementInfoNoViewId(LiveActor* pActor, const ActorInitInfo& rInfo) {
    PlacementInfo placementInfo;
    ActorInitInfo info;
    info.initNoViewId(&placementInfo, rInfo);
    pActor->init(info);
}

/**
 * Creates and initializes the actor of a placement using a factory.
 * @param rFactory The actor factory.
 * @param rInfo The actor init info.
 * @param pPlacementInfo The placement info.
 * @return The actor, or nullptr if the class is unknown.
 */
LiveActor* createPlacementActorFromFactory(const ActorFactory& rFactory, const ActorInitInfo& rInfo,
                                           const PlacementInfo* pPlacementInfo) {
    ActorInitInfo info;
    info.initViewIdSelf(pPlacementInfo, rInfo);
    return createActorFromFactory(rFactory, info);
}

/**
 * Creates and initializes the actor of a link using a factory.
 * @param rFactory The actor factory.
 * @param rInfo The actor init info.
 * @param pLinkName The link name.
 * @param index The link index.
 * @return The actor, or nullptr if the class is unknown.
 */
LiveActor* createLinksActorFromFactory(const ActorFactory& rFactory, const ActorInitInfo& rInfo,
                                       const char* pLinkName, s32 index) {
    ActorInitInfo info;
    PlacementInfo placementInfo;
    getLinksInfoByIndex(&placementInfo, *rInfo.mPlacementInfo, pLinkName, index);
    info.initViewIdSelf(&placementInfo, rInfo);
    return createActorFromFactory(rFactory, info);
}

/**
 * Creates the nerve keeper of an actor.
 * @param pActor The actor.
 * @param pNerve The first nerve.
 * @param maxStates The maximum number of nerve states.
 */
void initNerve(LiveActor* pActor, const Nerve* pNerve, s32 maxStates) {
    pActor->initNerveKeeper(new NerveKeeper(pActor, pNerve, maxStates));
}

/**
 * Creates the nerve keeper of an actor driven by nerve actions.
 * @param pActor The actor.
 * @param pActionName The first action.
 * @param pCollector The nerve action collector.
 * @param maxStates The maximum number of nerve states.
 */
void initNerveAction(LiveActor* pActor, const char* pActionName,
                     alNerveFunction::NerveActionCollector* pCollector, s32 maxStates) {
    NerveActionCtrl* actionCtrl = new NerveActionCtrl(pCollector);
    initNerve(pActor, actionCtrl->findNerve(pActionName), maxStates);
    pActor->getNerveKeeper()->initNerveAction(actionCtrl);
    startNerveAction(pActor, pActionName);
}

/**
 * Makes an actor appear and die with its stage switch.
 * @param pActor The actor.
 * @return Whether the actor listens to a stage switch.
 */
bool trySyncStageSwitchAppear(LiveActor* pActor) {
    using LiveActorFunctor = FunctorV0M<LiveActor*, void (LiveActor::*)()>;

    if (listenStageSwitchOnOffAppear(pActor, LiveActorFunctor(pActor, &LiveActor::appear),
                                     LiveActorFunctor(pActor, &LiveActor::kill))) {
        pActor->makeActorDead();
        return true;
    }

    pActor->makeActorAppeared();
    return false;
}

/**
 * Makes an actor die and appear with its stage switch.
 * @param pActor The actor.
 * @return Whether the actor listens to a stage switch.
 */
bool trySyncStageSwitchKill(LiveActor* pActor) {
    using LiveActorFunctor = FunctorV0M<LiveActor*, void (LiveActor::*)()>;
    bool isListen = listenStageSwitchOnOffKill(pActor, LiveActorFunctor(pActor, &LiveActor::kill),
                                               LiveActorFunctor(pActor, &LiveActor::appear));
    pActor->makeActorAppeared();
    return isListen;
}

/**
 * Syncs the appearance of an actor with its appear or kill stage switch.
 * @param pActor The actor.
 * @return Whether the actor listens to a stage switch.
 */
bool trySyncStageSwitchAppearAndKill(LiveActor* pActor) {
    if (trySyncStageSwitchAppear(pActor)) {
        return true;
    }

    return trySyncStageSwitchKill(pActor);
}

/**
 * Makes an actor appear when its stage switch turns on.
 * @param pActor The actor.
 * @return Whether the actor listens to a stage switch.
 */
bool tryListenStageSwitchAppear(LiveActor* pActor) {
    using LiveActorFunctor = FunctorV0M<LiveActor*, void (LiveActor::*)()>;

    if (listenStageSwitchOnAppear(pActor, LiveActorFunctor(pActor, &LiveActor::appear))) {
        pActor->makeActorDead();
        return true;
    }

    pActor->makeActorAppeared();
    return false;
}

/**
 * Makes an actor die when its stage switch turns on.
 * @param pActor The actor.
 * @return Whether the actor listens to a stage switch.
 */
bool tryListenStageSwitchKill(LiveActor* pActor) {
    using LiveActorFunctor = FunctorV0M<LiveActor*, void (LiveActor::*)()>;
    return listenStageSwitchOnKill(pActor, LiveActorFunctor(pActor, &LiveActor::kill));
}

/**
 * Scales the sensors of an actor by its y scale.
 * @param pActor The actor.
 */
void syncSensorScaleY(LiveActor* pActor) {
    f32 scaleY = getScale(pActor).y;
    s32 sensorNum = pActor->mHitSensorKeeper->mSensorCount;

    for (s32 i = 0; i < sensorNum; i++) {
        setSensorRadius(pActor, i, scaleY * getSensorRadius(pActor, i));
        sead::Vector3f offset = {getSensorFollowPosOffset(pActor, i).x,
                                 scaleY * getSensorFollowPosOffset(pActor, i).y,
                                 getSensorFollowPosOffset(pActor, i).z};
        setSensorFollowPosOffset(pActor, i, offset);
    }
}

/**
 * Scales the sensors and collider of an actor by its y scale.
 * @param pActor The actor.
 */
void syncSensorAndColliderScaleY(LiveActor* pActor) {
    syncSensorScaleY(pActor);
    f32 scaleY = getScale(pActor).y;
    setColliderRadius(pActor, scaleY * getColliderRadius(pActor));
    setColliderOffsetY(pActor, scaleY * getColliderOffsetY(pActor));
}

/**
 * Sets the material code used by the effects and sounds of an actor.
 * @param pActor The actor.
 * @param pMaterialCode The material code.
 */
void setMaterialCode(LiveActor* pActor, const char* pMaterialCode) {
    if (pActor->getEffectKeeper() != nullptr) {
        tryUpdateEffectMaterialCode(pActor, pMaterialCode);
    }

    if (pActor->getAudioKeeper() != nullptr) {
        tryUpdateSeMaterialCode(pActor, pMaterialCode);
    }
}

/**
 * Moves an actor by its placed display offset.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @return Whether a display offset is placed.
 */
bool tryAddDisplayOffset(LiveActor* pActor, const ActorInitInfo& rInfo) {
    sead::Vector3f offset = {0.0f, 0.0f, 0.0f};

    if (!tryGetDisplayOffset(&offset, rInfo)) {
        return false;
    }

    *getTransPtr(pActor) += offset;
    return true;
}

/**
 * Scales an actor by its placed display scale.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @return Whether a display scale is placed.
 */
bool tryAddDisplayScale(LiveActor* pActor, const ActorInitInfo& rInfo) {
    sead::Vector3f scale = {0.0f, 0.0f, 0.0f};

    if (!tryGetDisplayScale(&scale, rInfo)) {
        return false;
    }

    setScaleX(pActor, getScale(pActor).x * scale.x);
    setScaleY(pActor, getScale(pActor).y * scale.y);
    setScaleZ(pActor, getScale(pActor).z * scale.z);
    return true;
}

/**
 * Gets the placement info of init info.
 * @param rInfo The actor init info.
 * @return The placement info.
 */
const PlacementInfo& getPlacementInfo(const ActorInitInfo& rInfo) {
    return *rInfo.mPlacementInfo;
}

/**
 * Gets the layout init info of init info.
 * @param rInfo The actor init info.
 * @return The layout init info.
 */
const LayoutInitInfo& getLayoutInitInfo(const ActorInitInfo& rInfo) {
    return *rInfo.mLayoutInitInfo;
}

/**
 * Gets the audio director of init info.
 * @param rInfo The actor init info.
 * @return The audio director.
 */
AudioDirector* getAudioDirector(const ActorInitInfo& rInfo) {
    return rInfo.mAudioDirector;
}

/**
 * Reads a float from a model resource yaml.
 * @param pValue The value.
 * @param pActor The actor.
 * @param pFileName The yaml name.
 * @param pKey The key.
 */
void getActorRecourseDataF32(f32* pValue, LiveActor* pActor, const char* pFileName,
                             const char* pKey) {
    isExistModelResourceYaml(pActor, pFileName, nullptr);
    ByamlIter iter(getModelResourceYaml(pActor, pFileName, nullptr));
    tryGetByamlF32(pValue, iter, pKey);
}

/**
 * Reads a string from a model resource yaml.
 * @param pValue The value.
 * @param pActor The actor.
 * @param pFileName The yaml name.
 * @param pKey The key.
 */
void getActorRecourseDataString(const char** pValue, LiveActor* pActor, const char* pFileName,
                                const char* pKey) {
    isExistModelResourceYaml(pActor, pFileName, nullptr);
    ByamlIter iter(getModelResourceYaml(pActor, pFileName, nullptr));
    *pValue = getByamlKeyString(iter, pKey);
}

/**
 * Reads a vector from a model resource yaml.
 * @param pValue The value.
 * @param pActor The actor.
 * @param pFileName The yaml name.
 * @param pKey The key, or nullptr to read the root.
 */
void getActorRecourseDataV3f(sead::Vector3f* pValue, LiveActor* pActor, const char* pFileName,
                             const char* pKey) {
    isExistModelResourceYaml(pActor, pFileName, nullptr);
    ByamlIter iter(getModelResourceYaml(pActor, pFileName, nullptr));

    if (pKey != nullptr) {
        tryGetByamlV3f(pValue, iter, pKey);
    } else {
        tryGetByamlV3f(pValue, iter);
    }
}

/**
 * Reads a box from a model resource yaml.
 * @param pValue The value.
 * @param pActor The actor.
 * @param pFileName The yaml name.
 * @param pKey The key, or nullptr to read the root.
 */
void getActorRecourseDataBox3f(sead::BoundBox3f* pValue, LiveActor* pActor, const char* pFileName,
                               const char* pKey) {
    isExistModelResourceYaml(pActor, pFileName, nullptr);
    ByamlIter iter(getModelResourceYaml(pActor, pFileName, nullptr));

    if (pKey != nullptr) {
        tryGetByamlBox3f(pValue, iter, pKey);
    } else {
        tryGetByamlBox3f(pValue, iter);
    }
}

/**
 * Initializes a map parts actor from an archive without a placement.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pArchiveName The archive name.
 */
void initMapPartsActorNoPlacementInfo(LiveActor* pActor, const ActorInitInfo& rInfo,
                                      const char* pArchiveName) {
    PlacementInfo placementInfo;
    ActorInitInfo info;
    info.initViewIdHost(&placementInfo, rInfo);
    initActorImpl(pActor, info, "ObjectData", pArchiveName, nullptr, 0);
}
}  // namespace al
