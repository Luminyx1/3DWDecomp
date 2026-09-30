#include "Library/LiveActor/SubActorKeeper.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Obj/BreakModel.hpp"
#include "Library/Obj/DepthShadowModel.hpp"
#include "Library/Obj/ModelDrawParts.hpp"
#include "Library/Obj/PartsModel.hpp"
#include "Library/Obj/SilhouetteModel.hpp"
#include "Library/Obj/SimpleCircleShadowXZ.hpp"
#include "Library/Obj/WarpedMtxPartsModel.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates a sub actor keeper if the actor has a sub actor init file or reserves sub actors.
 * @param pRootActor The actor owning the sub actors.
 * @param rInfo The actor init info.
 * @param pSuffix The init file suffix.
 * @param maxSubActors The number of sub actors to reserve.
 * @return The keeper, or nullptr.
 */
SubActorKeeper* SubActorKeeper::tryCreate(LiveActor* pRootActor, const ActorInitInfo& rInfo,
                                          const char* pSuffix, s32 maxSubActors) {
    StringTmp<128> fileName;

    if (isExistModelResource(pRootActor)) {
        bool isExistFile = tryGetActorInitFileName(&fileName, pRootActor, "InitSubActor", pSuffix);

        if (!isExistFile && maxSubActors <= 0) {
            return nullptr;
        }

        if (maxSubActors <= 0 && !(isExistModelResource(pRootActor) &&
                                   isExistModelResourceYaml(pRootActor, fileName.cstr(), nullptr))) {
            return nullptr;
        }
    } else if (maxSubActors <= 0) {
        return nullptr;
    }

    return new SubActorKeeper(pRootActor, rInfo, pSuffix, maxSubActors);
}

/**
 * Creates a sub actor keeper without reading an init file.
 * @param pRootActor The actor owning the sub actors.
 * @param rInfo The actor init info.
 * @param maxSubActors The number of sub actors to reserve.
 * @return The keeper.
 */
SubActorKeeper* SubActorKeeper::createNoFile(LiveActor* pRootActor, const ActorInitInfo& rInfo,
                                             s32 maxSubActors) {
    return new SubActorKeeper(pRootActor, rInfo, nullptr, maxSubActors);
}

/**
 * Registers a sub actor.
 * @param pSubActor The sub actor.
 * @param syncType The sync flags of the sub actor.
 */
void SubActorKeeper::registerSubActor(LiveActor* pSubActor, u32 syncType) {
    mInfos[mCount] = new SubActorInfo(pSubActor, syncType);
    mCount++;
}

/**
 * Creates the sub actors listed in the sub actor init file of an actor.
 * @param pRootActor The actor owning the sub actors.
 * @param rInfo The actor init info.
 * @param pSuffix The init file suffix.
 * @param maxSubActors The number of extra sub actors to reserve.
 */
SubActorKeeper::SubActorKeeper(LiveActor* pRootActor, const ActorInitInfo& rInfo,
                               const char* pSuffix, s32 maxSubActors)
    : mRootActor(pRootActor) {
    StringTmp<128> fileName;

    if (isExistModelResource(pRootActor) &&
        !tryGetActorInitFileName(&fileName, pRootActor, "InitSubActor", pSuffix)) {
        createFileNameBySuffix(&fileName, "InitSubActor", pSuffix);
    }

    const u8* byml = nullptr;
    s32 creatorNum = 0;

    if (isExistModelResource(pRootActor) &&
        isExistModelResourceYaml(pRootActor, fileName.cstr(), nullptr)) {
        byml = getModelResourceYaml(mRootActor, fileName.cstr(), nullptr);
        ByamlIter iter(byml);
        ByamlIter initInfoIter;

        if (iter.tryGetIterByKey(&initInfoIter, "InitInfo")) {
            s32 addActorNum = 0;
            maxSubActors += initInfoIter.tryGetIntByKey(&addActorNum, "AddActorNum") ? addActorNum : 0;
        }

        ByamlIter creatorListIter;
        creatorNum =
            iter.tryGetIterByKey(&creatorListIter, "CreatorList") ? creatorListIter.getSize() : 0;
    }

    maxSubActors += creatorNum;
    mMaxCount = maxSubActors;
    mInfos = new SubActorInfo*[maxSubActors];

    for (s32 i = 0; i < mMaxCount; i++) {
        mInfos[i] = nullptr;
    }

    if (!byml) {
        return;
    }

    ByamlIter iter(byml);
    ByamlIter creatorListIter;
    iter.tryGetIterByKey(&creatorListIter, "CreatorList");

    for (s32 i = 0; i < creatorNum; i++) {
        ByamlIter creatorIter;
        creatorListIter.tryGetIterByIndex(&creatorIter, i);
        SubActorInfo* info = new SubActorInfo(nullptr, 0);
        mInfos[i] = info;
        LiveActor* rootActor = mRootActor;
        const char* objectName = tryGetByamlKeyStringOrNULL(creatorIter, "ObjectName");
        const char* modelName = tryGetByamlKeyStringOrNULL(creatorIter, "ModelName");
        const char* suffix = tryGetByamlKeyStringOrNULL(creatorIter, "InitFileSuffixName");
        const char* className = tryGetByamlKeyStringOrNULL(creatorIter, "ClassName");
        const char* categoryName = tryGetByamlKeyStringOrNULL(creatorIter, "CategoryName");
        bool isAlive = false;
        bool isExistAlive = creatorIter.tryGetBoolByKey(&isAlive, "IsAlive");
        bool isUseHostPlacementInfo = true;
        creatorIter.tryGetBoolByKey(&isUseHostPlacementInfo, "IsUseHostPlacementInfo");

        if (tryGetByamlKeyBoolOrFalse(creatorIter, "IsSyncAppear")) {
            info->mSyncType |= 1;
        }

        if (tryGetByamlKeyBoolOrFalse(creatorIter, "IsSyncClipping")) {
            info->mSyncType |= 2;
        }

        if (tryGetByamlKeyBoolOrFalse(creatorIter, "IsSyncHide")) {
            info->mSyncType |= 4;
        }

        if (!className) {
            LiveActor* actor = new LiveActor(objectName);
            info->mSubActor = actor;

            if (isUseHostPlacementInfo) {
                initActorWithArchiveName(actor, rInfo, modelName, suffix);
            } else {
                initActorWithArchiveNameNoPlacementInfo(actor, rInfo, modelName, suffix);
            }
        } else if (isEqualString(className, "PartsModel")) {
            const char* fixFileSuffix = tryGetByamlKeyStringOrNULL(creatorIter, "FixFileSuffixName");
            PartsModel* partsModel = new PartsModel(objectName);
            partsModel->initPartsFixFile(rootActor, rInfo, modelName, suffix, fixFileSuffix);
            info->mSubActor = partsModel;
        } else if (isEqualString(className, "WarpedMtxPartsModel")) {
            const char* fixFileSuffix = tryGetByamlKeyStringOrNULL(creatorIter, "FixFileSuffixName");
            WarpedMtxPartsModel* partsModel = new WarpedMtxPartsModel(objectName);
            partsModel->initPartsFixFile(rootActor, rInfo, modelName, suffix, fixFileSuffix);
            info->mSubActor = partsModel;
        } else if (isEqualString(className, "BreakModel")) {
            const char* actionName = tryGetByamlKeyStringOrNULL(creatorIter, "ActionName");
            const char* jointName = tryGetByamlKeyStringOrNULL(creatorIter, "JointName");
            const sead::Matrix34f* jointMtx =
                jointName ? getJointMtxPtr(rootActor, jointName) : nullptr;
            BreakModel* breakModel =
                new BreakModel(rootActor, objectName, modelName, suffix, jointMtx,
                               actionName ? actionName : "Break", false);
            initCreateActorNoPlacementInfo(breakModel, rInfo);
            info->mSubActor = breakModel;
        } else if (isEqualString(className, "SilhouetteModel")) {
            info->mSubActor = new SilhouetteModel(rootActor, rInfo, categoryName);
            info->mSyncType |= 8;
        } else if (isEqualString(className, "DepthShadowModel")) {
            info->mSubActor = new DepthShadowModel(rootActor, rInfo, categoryName);
            info->mSyncType |= 8;
        } else if (isEqualString(className, "InvincibleModel")) {
            info->mSubActor = new ModelDrawParts("無敵モデル", rootActor, rInfo, categoryName);
            info->mSyncType |= 8;
        } else if (isEqualString(className, "SimpleCircleShadowXZ")) {
            SimpleCircleShadowXZ* shadow = new SimpleCircleShadowXZ(objectName);
            shadow->initSimpleCircleShadow(rootActor, rInfo, modelName, suffix);
            info->mSubActor = shadow;
        }

        initActorModelForceCubeMap(info->mSubActor, rInfo);

        if (isExistAlive) {
            if (isAlive) {
                info->mSubActor->makeActorAppeared();
            } else {
                info->mSubActor->makeActorDead();
            }
        }

        mCount++;
    }
}
}  // namespace al
