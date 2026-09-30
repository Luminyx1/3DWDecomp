#include <cstring>

#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Action/Common/ActionAnimInfo.hpp"
#include "Project/Action/Common/InitResourceDataAction.hpp"

namespace al {

/**
 * Creates the action animation table if the resource has one.
 * @param pResource Resource to read from.
 * @param pDataAnim Animation data of the resource.
 * @return Created table, or nullptr.
 */
InitResourceDataActionAnim*
InitResourceDataActionAnim::tryCreate(Resource* pResource, const InitResourceDataAnim* pDataAnim) {
    if (!isExistResourceYaml(pResource, "ActionAnimCtrl", nullptr)) {
        return nullptr;
    }

    return new InitResourceDataActionAnim(pResource, pDataAnim);
}

static inline void loadActionAnimDataInfo(ActionAnimDataInfo* pData, const ByamlIter& rParentIter,
                                          ActionAnimCtrlInfo* pCtrlInfo, const char* pAnimType,
                                          ActionAnimType actionAnimType) {
    ByamlIter iter;

    if (rParentIter.tryGetIterByKey(&iter, pAnimType)) {
        iter.tryGetStringByKey(&pData->actionName, "Name");
        iter.tryGetBoolByKey(&pData->isKeepAnim, "KeepAnim");
        iter.tryGetBoolByKey(&pData->isActionAnim, "ActionAnim");

        if (pData->isActionAnim) {
            pCtrlInfo->actionAnimType = actionAnimType;
        }
    }
}

/**
 * Reads every action entry of the resource's action animation table.
 * @param pResource Resource to read from.
 * @param pDataAnim Animation data of the resource.
 */
InitResourceDataActionAnim::InitResourceDataActionAnim(Resource* pResource,
                                                       const InitResourceDataAnim* pDataAnim) {
    ByamlIter actionAnimCtrlIter(findResourceYaml(pResource, "ActionAnimCtrl", nullptr));

    mAnimInfoCount = actionAnimCtrlIter.getSize();
    mAnimInfos = new ActionAnimCtrlInfo*[mAnimInfoCount];

    for (s32 i = 0; i < mAnimInfoCount; i++) {
        ByamlIter iterIndex;
        actionAnimCtrlIter.tryGetIterByIndex(&iterIndex, i);

        ByamlIter sklIter;
        s32 sklNum = 1;

        if (iterIndex.tryGetIterByKey(&sklIter, "SklAnim") && sklIter.isTypeArray()) {
            sklNum = sklIter.getSize();
        }

        ActionAnimCtrlInfo* ctrlInfo = new ActionAnimCtrlInfo(sklNum);
        mAnimInfos[i] = ctrlInfo;
        iterIndex.tryGetStringByKey(&ctrlInfo->actionName, "ActionName");

        if (i == 0 && !ctrlInfo->actionName) {
            ctrlInfo->actionName = getResourceName(pResource);
        }

        if (iterIndex.tryGetIterByKey(&sklIter, "SklAnim")) {
            if (sklIter.isTypeArray()) {
                for (s32 e = 0; e < sklNum; e++) {
                    ActionAnimDataInfo* sklDatas = ctrlInfo->sklDatas;
                    ByamlIter animIter;
                    sklIter.tryGetIterByIndex(&animIter, e);
                    animIter.tryGetStringByKey(&sklDatas[e].actionName, "Name");
                }
            } else {
                ActionAnimDataInfo* sklData = ctrlInfo->sklDatas;
                sklIter.tryGetStringByKey(&sklData->actionName, "Name");
                sklIter.tryGetBoolByKey(&sklData->isKeepAnim, "KeepAnim");
                sklIter.tryGetBoolByKey(&sklData->isActionAnim, "ActionAnim");

                if (sklData->isActionAnim) {
                    ctrlInfo->actionAnimType = ActionAnimType::Skl;
                }
            }
        }

        loadActionAnimDataInfo(&ctrlInfo->mtpData, iterIndex, ctrlInfo, "MtpAnim",
                               ActionAnimType::Mtp);
        loadActionAnimDataInfo(&ctrlInfo->mclData, iterIndex, ctrlInfo, "MclAnim",
                               ActionAnimType::Mcl);
        loadActionAnimDataInfo(&ctrlInfo->mtsData, iterIndex, ctrlInfo, "MtsAnim",
                               ActionAnimType::Mts);
        loadActionAnimDataInfo(&ctrlInfo->visData, iterIndex, ctrlInfo, "VisAnim",
                               ActionAnimType::Vis);
    }

    sortCtrlInfo();
}

/**
 * Sorts the action entries after the first one by name.
 */
void InitResourceDataActionAnim::sortCtrlInfo() {
    for (s32 i = 1; i < mAnimInfoCount - 1; i++) {
        for (s32 e = i + 1; e < mAnimInfoCount; e++) {
            if (strcmp(mAnimInfos[i]->actionName, mAnimInfos[e]->actionName) > 0) {
                ActionAnimCtrlInfo* tmp = mAnimInfos[i];
                mAnimInfos[i] = mAnimInfos[e];
                mAnimInfos[e] = tmp;
            }
        }
    }
}

}  // namespace al
