#include "Project/Action/Common/ActionAnimCtrl.hpp"

#include <cstring>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/SaveData/ActorInitResourceData.hpp"
#include "Project/Action/Common/ActionAnimInfo.hpp"
#include "Project/Action/Common/InitResourceDataAction.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Creates an animation controller if the actor has an action animation table.
 * @param pActor Actor to animate.
 * @param pArchiveName Name of the actor's archive.
 * @param pSuffix Suffix of the table file.
 * @return Created controller, or nullptr.
 */
ActionAnimCtrl* ActionAnimCtrl::tryCreate(LiveActor* pActor, const char* pArchiveName,
                                          const char* pSuffix) {
    if (!pActor->mModelKeeper) {
        return nullptr;
    }

    StringTmp<128> fileName;
    if (!tryGetActorInitFileName(&fileName, pActor, "ActionAnimCtrl", pSuffix) &&
        !tryGetActorAnimInitFileName(&fileName, pActor, "ActionAnimCtrl", pSuffix)) {
        createFileNameBySuffix(&fileName, "ActionAnimCtrl", pSuffix);
    }

    if (!isExistModelOrAnimResourceYaml(pActor, fileName.cstr(), nullptr)) {
        return nullptr;
    }

    ActionAnimCtrl* ctrl = new ActionAnimCtrl(pActor);
    ctrl->init(pArchiveName, pSuffix);
    return ctrl;
}

void ActionAnimCtrl::init(const char* pArchiveName, const char* pSuffix) {
    mArchiveName = pArchiveName;
    InitResourceDataActionAnim* dataActionAnim =
        reinterpret_cast<ActorInitResourceData*>(getAnimResource(mParentActor)->_B0)
            ->getDataAction()
            ->getDataActionAnim();
    mInfoCount = dataActionAnim->getAnimInfoCount();

    if (getAnimResource(mParentActor) == getModelResource(mParentActor)) {
        mInfos = dataActionAnim->getAnimInfos();
        return;
    }

    mInfos = new ActionAnimCtrlInfo*[mInfoCount];

    const char* archiveName = mArchiveName;
    const ActionAnimCtrlInfo* srcInfo = dataActionAnim->getAnimInfos()[0];
    ActionAnimCtrlInfo* info = new ActionAnimCtrlInfo(srcInfo->sklDataCount);
    info->actionName = createStringIfInStack(archiveName);
    for (s32 i = 0; i < srcInfo->sklDataCount; i++) {
        info->sklDatas[i] = srcInfo->sklDatas[i];
    }
    info->mclData = srcInfo->mclData;
    info->mtsData = srcInfo->mtsData;
    info->mtpData = srcInfo->mtpData;
    info->visData = srcInfo->visData;
    info->actionAnimType = srcInfo->actionAnimType;
    mInfos[0] = info;

    for (s32 i = 1; i < mInfoCount; i++) {
        mInfos[i] = dataActionAnim->getAnimInfos()[i];
    }
}

bool ActionAnimCtrl::start(const char* pActionName) {
    mAnimType = -1;
    ActionAnimCtrlInfo* info = findAnimInfo(pActionName);
    mPlayingInfo = info;
    if (!info) {
        return false;
    }

    mAnimType = static_cast<s32>(info->actionAnimType);

    ActionAnimDataInfo* sklData = info->sklDatas;
    const char* sklAnimName = alActionFunction::getAnimName(info, sklData);
    if (isSklAnimExist(mParentActor, sklAnimName) &&
        !(sklData->isKeepAnim && isSklAnimPlaying(mParentActor, sklAnimName, 0))) {
        if (info->sklDataCount == 1) {
            bool isSameName = isEqualString(sklAnimName, info->actionName);
            LiveActor* actor = mParentActor;
            const char* animName = alActionFunction::getAnimName(info, info->sklDatas);
            if (isSameName) {
                startSklAnim(actor, animName);
            } else {
                startSklAnimInterpole(actor, animName, info->actionName);
            }
        } else {
            const char* animNames[6] = {};
            for (s32 i = 0; i < info->sklDataCount; i++) {
                animNames[i] = alActionFunction::getAnimName(info, &info->sklDatas[i]);
            }
            if (isEqualString(sklAnimName, info->actionName)) {
                startSklAnimBlend(mParentActor, animNames[0], animNames[1], animNames[2],
                                  animNames[3], animNames[4], animNames[5]);
            } else {
                startSklAnimBlendInterpole(mParentActor, info->actionName, animNames[0],
                                           animNames[1], animNames[2], animNames[3],
                                           animNames[4], animNames[5]);
            }
        }
        if (mAnimType == -1) {
            mAnimType = static_cast<s32>(ActionAnimType::Skl);
        }
    }

    const char* mtpAnimName = alActionFunction::getAnimName(info, &info->mtpData);
    const char* mclAnimName = alActionFunction::getAnimName(info, &info->mclData);
    const char* mtsAnimName = alActionFunction::getAnimName(info, &info->mtsData);
    const char* visAnimName = alActionFunction::getAnimName(info, &info->visData);

    if (isMtpAnimExist(mParentActor, mtpAnimName) &&
        !(info->mtpData.isKeepAnim && isMtpAnimPlaying(mParentActor, mtpAnimName))) {
        startMtpAnim(mParentActor, mtpAnimName);
        if (mAnimType == -1 && isEqualString(mtpAnimName, pActionName)) {
            mAnimType = static_cast<s32>(ActionAnimType::Mtp);
        }
    }
    if (isMclAnimExist(mParentActor, mclAnimName) &&
        !(info->mclData.isKeepAnim && isMclAnimPlaying(mParentActor, mclAnimName))) {
        startMclAnim(mParentActor, mclAnimName);
        if (mAnimType == -1 && isEqualString(mclAnimName, pActionName)) {
            mAnimType = static_cast<s32>(ActionAnimType::Mcl);
        }
    }
    if (isMtsAnimExist(mParentActor, mtsAnimName) &&
        !(info->mtsData.isKeepAnim && isMtsAnimPlaying(mParentActor, mtsAnimName))) {
        startMtsAnim(mParentActor, mtsAnimName);
        if (mAnimType == -1 && isEqualString(mtsAnimName, pActionName)) {
            mAnimType = static_cast<s32>(ActionAnimType::Mts);
        }
    }
    if (isVisAnimExist(mParentActor, visAnimName) &&
        !(info->visData.isKeepAnim && isVisAnimPlaying(mParentActor, visAnimName))) {
        startVisAnim(mParentActor, visAnimName);
        if (mAnimType == -1 && isEqualString(visAnimName, pActionName)) {
            mAnimType = static_cast<s32>(ActionAnimType::Vis);
        }
    }
    return true;
}

ActionAnimCtrlInfo* ActionAnimCtrl::findAnimInfo(const char* pActionName) const {
    s32 count = mInfoCount;
    if (count < 1) {
        return nullptr;
    }

    ActionAnimCtrlInfo* info = mInfos[0];
    if (strcmp(info->actionName, pActionName) == 0) {
        return info;
    }

    s32 lo = 1;
    s32 hi = count - 1;
    while (lo <= hi) {
        s32 mid = (lo + hi) >> 1;
        info = mInfos[mid];
        s32 result = strcmp(info->actionName, pActionName);
        if (result > 0) {
            hi = mid - 1;
        } else if (result < 0) {
            lo = mid + 1;
        } else {
            return info;
        }
    }
    return nullptr;
}

/**
 * Gets the frame of the animation driving the current action.
 * @return Current frame.
 */
f32 ActionAnimCtrl::getFrame() const {
    return alAnimFunction::getAllAnimFrame(mParentActor, mAnimType);
}

f32 ActionAnimCtrl::getActionFrameMax(const char* pActionName) const {
    ActionAnimCtrlInfo* info = findAnimInfo(pActionName);
    if (!info) {
        return alAnimFunction::getAllAnimFrameMax(mParentActor, pActionName, -1);
    }

    switch (info->actionAnimType) {
    case ActionAnimType::None: {
        for (s32 i = 0; i < info->sklDataCount; i++) {
            const char* animName = alActionFunction::getAnimName(info, &info->sklDatas[i]);
            if (isSklAnimExist(mParentActor, animName)) {
                return getSklAnimFrameMax(mParentActor, animName);
            }
        }
        const char* mclAnimName = alActionFunction::getAnimName(info, &info->mclData);
        if (isMclAnimExist(mParentActor, mclAnimName)) {
            return getMclAnimFrameMax(mParentActor, mclAnimName);
        }
        const char* mtpAnimName = alActionFunction::getAnimName(info, &info->mtpData);
        if (isMtpAnimExist(mParentActor, mtpAnimName)) {
            return getMtpAnimFrameMax(mParentActor, mtpAnimName);
        }
        const char* mtsAnimName = alActionFunction::getAnimName(info, &info->mtsData);
        if (isMtsAnimExist(mParentActor, mtsAnimName)) {
            return getMtsAnimFrameMax(mParentActor, mtsAnimName);
        }
        const char* visAnimName = alActionFunction::getAnimName(info, &info->visData);
        if (isVisAnimExist(mParentActor, visAnimName)) {
            return getVisAnimFrameMax(mParentActor, visAnimName);
        }
        return 1.0f;
    }
    case ActionAnimType::Skl:
        for (s32 i = 0; i < info->sklDataCount; i++) {
            const char* animName = alActionFunction::getAnimName(info, &info->sklDatas[i]);
            if (isSklAnimExist(mParentActor, animName)) {
                return getSklAnimFrameMax(mParentActor, animName);
            }
        }
        return 0.0f;
    case ActionAnimType::Mcl:
        return getMclAnimFrameMax(mParentActor,
                                  alActionFunction::getAnimName(info, &info->mclData));
    case ActionAnimType::Mtp:
        return getMtpAnimFrameMax(mParentActor,
                                  alActionFunction::getAnimName(info, &info->mtpData));
    case ActionAnimType::Mts:
        return getMtsAnimFrameMax(mParentActor,
                                  alActionFunction::getAnimName(info, &info->mtsData));
    case ActionAnimType::Vis:
        return getVisAnimFrameMax(mParentActor,
                                  alActionFunction::getAnimName(info, &info->visData));
    default:
        return 1.0f;
    }
}

/**
 * Gets the frame rate of the animation driving the current action.
 * @return Current frame rate.
 */
f32 ActionAnimCtrl::getFrameRate() const {
    return alAnimFunction::getAllAnimFrameRate(mParentActor, mAnimType);
}

/**
 * Checks whether an action is playing; the frame itself is not changed.
 * @param frame Requested frame.
 * @return Whether an action is playing.
 */
bool ActionAnimCtrl::trySetFrame(f32 frame) {
    return mPlayingInfo != nullptr;
}

bool ActionAnimCtrl::isExistAction(const char* pActionName) const {
    return findAnimInfo(pActionName) != nullptr;
}

bool ActionAnimCtrl::isActionOneTime(const char* pActionName) const {
    ActionAnimCtrlInfo* info = findAnimInfo(pActionName);
    if (!info) {
        if (isSklAnimExist(mParentActor, pActionName)) {
            return isSklAnimOneTime(mParentActor, pActionName);
        }
        if (isMtpAnimExist(mParentActor, pActionName)) {
            return isMtpAnimOneTime(mParentActor, pActionName);
        }
        if (isMclAnimExist(mParentActor, pActionName)) {
            return isMclAnimOneTime(mParentActor, pActionName);
        }
        if (isMtsAnimExist(mParentActor, pActionName)) {
            return isMtsAnimOneTime(mParentActor, pActionName);
        }
        if (isVisAnimExist(mParentActor, pActionName)) {
            return isVisAnimOneTime(mParentActor, pActionName);
        }
        return true;
    }

    switch (info->actionAnimType) {
    case ActionAnimType::None: {
        if (info->sklDataCount != 0) {
            const char* sklAnimName = alActionFunction::getAnimName(info, info->sklDatas);
            if (isSklAnimExist(mParentActor, sklAnimName)) {
                return isSklAnimOneTime(mParentActor, sklAnimName);
            }
        }
        const char* mtpAnimName = alActionFunction::getAnimName(info, &info->mtpData);
        if (isMtpAnimExist(mParentActor, mtpAnimName)) {
            return isMtpAnimOneTime(mParentActor, mtpAnimName);
        }
        const char* mclAnimName = alActionFunction::getAnimName(info, &info->mclData);
        if (isMclAnimExist(mParentActor, mclAnimName)) {
            return isMclAnimOneTime(mParentActor, mclAnimName);
        }
        const char* mtsAnimName = alActionFunction::getAnimName(info, &info->mtsData);
        if (isMtsAnimExist(mParentActor, mtsAnimName)) {
            return isMtsAnimOneTime(mParentActor, mtsAnimName);
        }
        const char* visAnimName = alActionFunction::getAnimName(info, &info->visData);
        if (isVisAnimExist(mParentActor, visAnimName)) {
            return isVisAnimOneTime(mParentActor, visAnimName);
        }
        break;
    }
    case ActionAnimType::Skl:
        return isSklAnimOneTime(mParentActor,
                                alActionFunction::getAnimName(info, info->sklDatas));
    case ActionAnimType::Mcl:
        return isMclAnimOneTime(mParentActor,
                                alActionFunction::getAnimName(info, &info->mclData));
    case ActionAnimType::Mtp:
        return isMtpAnimOneTime(mParentActor,
                                alActionFunction::getAnimName(info, &info->mtpData));
    case ActionAnimType::Mts:
        return isMtsAnimOneTime(mParentActor,
                                alActionFunction::getAnimName(info, &info->mtsData));
    case ActionAnimType::Vis:
        return isVisAnimOneTime(mParentActor,
                                alActionFunction::getAnimName(info, &info->visData));
    default:
        break;
    }
    return !isEqualString(pActionName, mArchiveName);
}

/**
 * Gets the name of the playing action.
 * @return Action name, or nullptr.
 */
const char* ActionAnimCtrl::getPlayingActionName() const {
    if (!mPlayingInfo) {
        return nullptr;
    }
    return mPlayingInfo->actionName;
}

/**
 * Constructs an empty controller.
 * @param pActor Actor to animate.
 */
ActionAnimCtrl::ActionAnimCtrl(LiveActor* pActor) : mParentActor(pActor) {}

/**
 * Sorts the action entries after the first one by name.
 */
void ActionAnimCtrl::sortCtrlInfo() {
    for (s32 i = 1; i < mInfoCount - 1; i++) {
        for (s32 e = i + 1; e < mInfoCount; e++) {
            if (strcmp(mInfos[i]->actionName, mInfos[e]->actionName) > 0) {
                ActionAnimCtrlInfo* tmp = mInfos[i];
                mInfos[i] = mInfos[e];
                mInfos[e] = tmp;
            }
        }
    }
}

/**
 * Constructs an empty animation entry.
 */
ActionAnimDataInfo::ActionAnimDataInfo() = default;

/**
 * Constructs an action entry with room for skeletal animations.
 * @param sklSize Number of skeletal animations.
 */
ActionAnimCtrlInfo::ActionAnimCtrlInfo(s32 sklSize) : sklDataCount(sklSize) {
    sklDatas = new ActionAnimDataInfo[sklSize];
}

}  // namespace al
