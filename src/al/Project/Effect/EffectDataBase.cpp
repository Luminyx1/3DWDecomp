#include "Project/Effect/EffectDataBase.hpp"

#include <container/seadObjArray.h>
#include <cstring>
#include <math/seadVector.h>

#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/EffectEmitter.hpp"
#include "Project/Effect/EffectInfo.hpp"

namespace {
struct HitReactionTmp {
    const char* mReactionName;
    sead::FixedObjArray<al::EffectHitReactionData, 8> mDatas;
};

const char* cMaterialTypePrefix[] = {"InWater", "Water", "Wet", "Shallow"};
}  // namespace

namespace al {

/**
 * Constructs a draw category with default settings.
 */
EffectDrawCategoryInfo::EffectDrawCategoryInfo()
    : mName(nullptr), mParticleNumMaxBit(6), mStripeVertexNumBit(6), mIsEnableZSort(false),
      mIsAlwaysUpdateUbo(false), mIsScreenEffect(false) {}

/**
 * Loads the draw categories and the effect user infos of an archive.
 * @param pArchiveName Archive name.
 */
EffectDataBase::EffectDataBase(const char* pArchiveName)
    : mUserNum(0), mUsers(nullptr), mDrawCategoryNum(0), mDrawCategories(nullptr) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    const u8* categoryByml = tryGetByml(resource, "DrawCategoryList");

    s32 categoryListNum;
    if (categoryByml != nullptr) {
        ByamlIter categoryIter(categoryByml);
        s32 categoryNum = categoryIter.getSize();
        mDrawCategories = new EffectDrawCategoryInfo[categoryNum];

        for (s32 i = 0; i < categoryNum; i++) {
            ByamlIter iter;
            categoryIter.tryGetIterByIndex(&iter, i);
            EffectDrawCategoryInfo* info = &mDrawCategories[i];
            info->mName = getByamlKeyString(iter, "CategoryName");
            tryGetByamlS32(&info->mParticleNumMaxBit, iter, "ParticleNumMaxBit");
            tryGetByamlS32(&info->mStripeVertexNumBit, iter, "StripeVertexNumBit");

            if (!tryGetByamlBool(&info->mIsEnableZSort, iter, "EnableZSort")) {
                info->mIsEnableZSort = false;
            }

            if (!tryGetByamlBool(&info->mIsAlwaysUpdateUbo, iter, "AlwaysUpdateUBO")) {
                info->mIsAlwaysUpdateUbo = false;
            }

            if (!tryGetByamlBool(&info->mIsScreenEffect, iter, "ScreenEffect")) {
                info->mIsScreenEffect = false;
            }
        }

        mDrawCategoryNum = categoryNum;
        categoryListNum = 1;
    } else {
        mDrawCategories = new EffectDrawCategoryInfo[1];
        mDrawCategories[0].mName = "エフェクト（３Ｄ）";
        mDrawCategoryNum = 1;
        categoryListNum = 0;
    }

    s32 entryNum = resource->getEntryNum("/");
    mUsers = new EffectUserInfo*[entryNum - categoryListNum];

    s32 i = 0;
    s32 userNum = 0;
    for (; i < entryNum; i++) {
        StringTmp<128> entryName;
        resource->getEntryName(&entryName, "/", i);

        if (isEqualSubString(entryName.cstr(), "DrawCategoryList")) {
            continue;
        }

        mUsers[userNum] = alEffectDataBaseFunction::createEffectUserInfo(this, resource, entryName);
        userNum++;
    }

    for (s32 i = 0; i < userNum - 1; i++) {
        for (s32 j = i + 1; j < userNum; j++) {
            EffectUserInfo* userA = mUsers[i];
            EffectUserInfo* userB = mUsers[j];

            if (strcmp(userA->mName, userB->mName) > 0) {
                mUsers[i] = userB;
                mUsers[j] = userA;
            }
        }
    }

    mUserNum = userNum;
}

}  // namespace al

namespace alEffectDataBaseFunction {

al::EffectUserInfo* createEffectUserInfo(const al::EffectDataBase* pDataBase,
                                         const al::Resource* pResource,
                                         const sead::SafeString& rFileName) {
    al::EffectUserInfo* userInfo = new al::EffectUserInfo();

    s32 nameLength = rFileName.calcLength() - 4;
    char* userName = new char[nameLength];
    userInfo->mName = userName;
    al::removeExtensionString(userName, nameLength, rFileName.cstr());

    al::ByamlIter rootIter(pResource->getByml(userInfo->mName));

    al::ByamlIter effectListIter;
    rootIter.tryGetIterByKey(&effectListIter, "EffectList");
    userInfo->mEffectNum = effectListIter.getSize();
    userInfo->mEffects = new al::EffectInfo[userInfo->mEffectNum];

    sead::FixedObjArray<HitReactionTmp, 32> hitReactions;

    {
        al::ByamlIter matrixListIter;
        if (rootIter.tryGetIterByKey(&matrixListIter, "MatrixList")) {
            s32 matrixNum = matrixListIter.getSize();

            if (matrixNum >= 1) {
                al::EffectNamedMtxList* mtxList = new al::EffectNamedMtxList();
                userInfo->mNamedMtxList = mtxList;
                mtxList->mNum = matrixNum;
                mtxList->mNames = new const char*[matrixNum];

                for (s32 i = 0; i < matrixNum; i++) {
                    matrixListIter.tryGetStringByIndex(&userInfo->mNamedMtxList->mNames[i], i);
                }
            }
        }
    }

    for (s32 i = 0; i < userInfo->mEffectNum; i++) {
        al::ByamlIter effectIter;
        effectListIter.tryGetIterByIndex(&effectIter, i);
        al::EffectInfo* info = &userInfo->mEffects[i];
        effectIter.tryGetStringByKey(&info->mName, "EffectName");

        al::ByamlIter resourceListIter;
        effectIter.tryGetIterByKey(&resourceListIter, "ResourceList");
        info->mEmitInfoNum = resourceListIter.getSize();
        info->mEmitInfos = new al::EffectResourceInfo[info->mEmitInfoNum];

        for (s32 j = 0; j < info->mEmitInfoNum; j++) {
            al::ByamlIter resourceIter;
            resourceListIter.tryGetIterByIndex(&resourceIter, j);
            al::EffectResourceInfo* resourceInfo = &info->mEmitInfos[j];
            resourceIter.tryGetStringByKey(&resourceInfo->mName, "ResourceName");

            if (!resourceIter.tryGetStringByKey(&resourceInfo->mMaterialName, "MaterialType")) {
                continue;
            }

            resourceInfo->_10 = resourceInfo->mMaterialName;
            resourceInfo->_18 = "";

            for (s32 k = 0; k < 4; k++) {
                if (al::isStartWithString(resourceInfo->mMaterialName, cMaterialTypePrefix[k])) {
                    resourceInfo->_10 =
                        resourceInfo->mMaterialName + strlen(cMaterialTypePrefix[k]);
                    resourceInfo->_18 = cMaterialTypePrefix[k == 0 ? 1 : k];
                    break;
                }
            }
        }

        al::EffectEmitInfo* param = &info->mParam;
        param->mIsEmitIgnoreRotate = effectIter.isExistKey("EmitIgnoreRotate");
        param->mIsEmitIgnoreScale = effectIter.isExistKey("EmitIgnoreScale");
        param->mIsBillboard = effectIter.isExistKey("EmitBillboard");
        param->mIsYBillboard = effectIter.isExistKey("EmitYBillboard");
        param->mIsFollowCamera = effectIter.isExistKey("EmitCamera");
        param->mIsReEmitOnClip = effectIter.isExistKey("DeleteAtClipping");
        param->mIsOneTimeFade = effectIter.isExistKey("OneTimeFade");
        param->mIsFollowPos = effectIter.isExistKey("FollowPos");
        param->mIsFollowMtx = effectIter.isExistKey("FollowMtx");
        effectIter.tryGetStringByKey(&param->mJointName, "JointName");
        param->mIsFollowTransOnEmit = effectIter.isExistKey("EmitTrans");
        param->mIsFollowRotateOnEmit = effectIter.isExistKey("EmitRotate");
        param->mIsFollowScaleOnEmit = effectIter.isExistKey("EmitScale");
        param->mIsFollowTrans = effectIter.isExistKey("FollowTrans");
        param->mIsFollowRotate = effectIter.isExistKey("FollowRotate");
        param->mIsFollowScale = effectIter.isExistKey("FollowScale");
        param->mIsKeepFrontX = effectIter.isExistKey("BindWorldAxisX");
        param->mIsKeepFrontY = effectIter.isExistKey("BindWorldAxisY");
        param->mIsKeepFrontZ = effectIter.isExistKey("BindWorldAxisZ");
        param->mIsNoEmitAtNoCollide = effectIter.isExistKey("NoEmitAtNoCollide");
        param->mIsFollowCameraFovy = effectIter.isExistKey("FollowScaleByFovy");
        param->mIsSnapshotCameraMode = effectIter.isExistKey("SnapshotFollowCamera");
        al::tryGetByamlV3f(&param->mOffsetTrans, effectIter, "PosOffset");
        al::tryGetByamlV3f(&param->mOffsetRotate, effectIter, "Rotate");
        effectIter.tryGetIntByKey(&param->mForceCalcFrame, "ForceCalcFrame");
        effectIter.tryGetIntByKey(&param->mHandleNum, "HandleNum");

        const char* categoryName = nullptr;
        s32 groupId = 0;
        if (effectIter.tryGetStringByKey(&categoryName, "DrawCategory")) {
            for (s32 k = 0; k < pDataBase->getDrawCategoryNum(); k++) {
                if (al::isEqualString(pDataBase->getDrawCategory(k).mName, categoryName)) {
                    groupId = k;
                    break;
                }
            }
        }

        param->mGroupId = groupId;
        param->mIsNeedProgramInfo = effectIter.isExistKey("NeedProgramInfo");
        param->mIsIgnoreJoint = effectIter.isExistKey("DirectPos");
        effectIter.tryGetFloatByKey(&param->mScale, "Scale");
        effectIter.tryGetFloatByKey(&param->mParticleScale, "ParticleScale");
        effectIter.tryGetFloatByKey(&param->mEmitRatio, "EmitRatio");
        effectIter.tryGetFloatByKey(&param->mFarClipDistance, "FarClipDistance");
        al::tryGetByamlColor(&param->mColor, effectIter, "Color");

        if (param->mOffsetTrans != sead::Vector3f::zero) {
            param->mIsAddOffsetTrans = true;
        }

        if (param->mOffsetRotate != sead::Vector3f::zero) {
            param->mIsAddOffsetRotate = true;
        }

        if (!(param->mColor == sead::Color4f::cWhite)) {
            param->mIsSetColor = true;
        }

        if (!effectIter.isExistKey("EmitTrans")) {
            if (param->mIsFollowPos) {
                param->mIsFollowTrans = true;
            }

            if (param->mIsFollowMtx) {
                param->mIsFollowTrans = true;
                param->mIsFollowRotate = true;
                param->mIsFollowScale = true;
            }

            if (!param->mIsEmitIgnoreRotate) {
                param->mIsFollowRotateOnEmit = true;
            }

            if (!param->mIsEmitIgnoreScale) {
                param->mIsFollowScaleOnEmit = true;
            }
        }

        al::ByamlIter actionListIter;
        if (effectIter.tryGetIterByKey(&actionListIter, "ActionList")) {
            info->mActionNum = actionListIter.getSize();
            info->mActions = new al::ActionEffectData[info->mActionNum];

            for (s32 k = 0; k < info->mActionNum; k++) {
                al::ByamlIter actionIter;
                actionListIter.tryGetIterByIndex(&actionIter, k);
                al::ActionEffectData* action = &info->mActions[k];
                actionIter.tryGetStringByKey(&action->mActionName, "ActionName");
                actionIter.tryGetIntByKey(&action->mStartFrame, "StartFrame");
                actionIter.tryGetIntByKey(&action->mEndFrame, "EndFrame");
                actionIter.tryGetBoolByKey(&action->mIsKeepEmitter, "KeepEmitter");
            }
        }

        al::ByamlIter hitReactionListIter;
        if (effectIter.tryGetIterByKey(&hitReactionListIter, "HitReactionList")) {
            info->mHitReactionNum = hitReactionListIter.getSize();

            for (s32 k = 0; k < info->mHitReactionNum; k++) {
                al::ByamlIter hitReactionIter;
                hitReactionListIter.tryGetIterByIndex(&hitReactionIter, k);
                const char* reactionName;
                hitReactionIter.tryGetStringByKey(&reactionName, "ReactionName");

                HitReactionTmp* hitReaction = nullptr;
                for (s32 l = 0; l < hitReactions.size(); l++) {
                    if (al::isEqualString(hitReactions.unsafeAt(l)->mReactionName,
                                          reactionName)) {
                        hitReaction = hitReactions.unsafeAt(l);
                        break;
                    }
                }

                if (hitReaction == nullptr) {
                    hitReaction = hitReactions.emplaceBack();
                    hitReaction->mReactionName = reactionName;
                }

                al::EffectHitReactionData* data = hitReaction->mDatas.emplaceBack();
                data->mEffectName = info->mName;
                hitReactionIter.tryGetFloatByKey(&data->mPosOffsetBetweenSensors,
                                                 "PosOffsetBetweenSensors");
            }
        }

        userInfo->mHitReactionNum = hitReactions.size();
        if (userInfo->mHitReactionNum >= 1) {
            userInfo->mHitReactions = new al::EffectHitReactionInfo[userInfo->mHitReactionNum];

            for (s32 k = 0; k < userInfo->mHitReactionNum; k++) {
                HitReactionTmp* hitReaction = hitReactions.unsafeAt(k);
                al::EffectHitReactionInfo* hitReactionInfo = &userInfo->mHitReactions[k];
                hitReactionInfo->mReactionName = hitReaction->mReactionName;
                hitReactionInfo->mDataNum = hitReaction->mDatas.size();
                hitReactionInfo->mDatas = new al::EffectHitReactionData[hitReaction->mDatas.size()];

                for (s32 l = 0; l < hitReaction->mDatas.size(); l++) {
                    hitReactionInfo->mDatas[l] = *hitReaction->mDatas.at(l);
                }
            }
        }
    }

    return userInfo;
}

}  // namespace alEffectDataBaseFunction
