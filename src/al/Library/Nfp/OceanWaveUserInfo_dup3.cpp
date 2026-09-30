#include "Project/OceanWave/OceanWaveUserInfo.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/OceanWave/OceanWaveDirector.hpp"
#include "Project/OceanWave/OceanWaveKeeper.hpp"
#include "Project/Play/Actor/ActorAlphaCtrl.hpp"

namespace al {
/**
 * Creates ocean wave information from BYAML data.
 * @param rIter BYAML data
 * @return created information, or nullptr if the data has no name
 */
OceanWaveInfo* OceanWaveInfo::createInfo(const ByamlIter& rIter) {
    OceanWaveInfo* info = new OceanWaveInfo;
    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }

    rIter.tryGetStringByKey(&info->mJointName, "JointName");
    tryGetByamlV3f(&info->mPosOffset, rIter, "PosOffset");
    rIter.tryGetFloatByKey(&info->mSize, "Size");
    rIter.tryGetFloatByKey(&info->mSpeed, "Speed");
    rIter.tryGetFloatByKey(&info->mTime, "Time");
    rIter.tryGetFloatByKey(&info->mAmp, "Amp");
    rIter.tryGetFloatByKey(&info->mLen, "Len");
    return info;
}

/**
 * Compares two ocean wave information by name.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWaveInfo::compareInfo(const OceanWaveInfo* pA, const OceanWaveInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Creates ocean wave play information of an action from BYAML data.
 * @param rIter BYAML data
 * @return created information
 */
OceanWavePlayInfoInAction* OceanWavePlayInfoInAction::createInfo(const ByamlIter& rIter) {
    OceanWavePlayInfoInAction* info = new OceanWavePlayInfoInAction;
    rIter.tryGetStringByKey(&info->mName, "Name");
    if (!rIter.tryGetFloatByKey(&info->mStartFrame, "StartFrame")) {
        info->mStartFrame = 0.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mEndFrame, "EndFrame")) {
        info->mEndFrame = 0.0f;
    }

    return info;
}

/**
 * Compares two ocean wave play information of an action by start frame.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWavePlayInfoInAction::compareInfo(const OceanWavePlayInfoInAction* pA,
                                           const OceanWavePlayInfoInAction* pB) {
    if (pA->mStartFrame < pB->mStartFrame) {
        return -1;
    }

    return pA->mStartFrame > pB->mStartFrame;
}

/**
 * Constructs empty ocean wave play information of an action.
 */
OceanWavePlayInfoInAction::OceanWavePlayInfoInAction() = default;

/**
 * Copies ocean wave play information of an action.
 * @param rOther information to copy
 */
OceanWavePlayInfoInAction::OceanWavePlayInfoInAction(const OceanWavePlayInfoInAction& rOther)
    : mName(rOther.mName), mStartFrame(rOther.mStartFrame), mEndFrame(rOther.mEndFrame) {}

/**
 * Copies ocean wave play information of an action.
 * @param rOther information to copy
 * @return this information
 */
OceanWavePlayInfoInAction& OceanWavePlayInfoInAction::operator=(
    const OceanWavePlayInfoInAction& rOther) {
    mName = rOther.mName;
    mStartFrame = rOther.mStartFrame;
    mEndFrame = rOther.mEndFrame;
    return *this;
}

/**
 * Creates ocean wave action information from BYAML data.
 * @param rIter BYAML data
 * @return created information
 */
OceanWaveActionInfo* OceanWaveActionInfo::createInfo(const ByamlIter& rIter) {
    OceanWaveActionInfo* info = new OceanWaveActionInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter iter;
    rIter.tryGetIterByKey(&iter, "PlayInfoInActionList");
    info->mPlayInfoList = createInfoList<OceanWavePlayInfoInAction>(iter);
    return info;
}

/**
 * Compares two ocean wave action information by name.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWaveActionInfo::compareInfo(const OceanWaveActionInfo* pA, const OceanWaveActionInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Constructs empty ocean wave action information.
 */
OceanWaveActionInfo::OceanWaveActionInfo() : mName(nullptr), mPlayInfoList(nullptr) {}

/**
 * Copies ocean wave action information including its play information.
 * @param rOther information to copy
 */
OceanWaveActionInfo::OceanWaveActionInfo(const OceanWaveActionInfo& rOther)
    : mName(rOther.mName), mPlayInfoList(nullptr) {
    if (rOther.mPlayInfoList == nullptr) {
        mPlayInfoList = nullptr;
        return;
    }

    s32 num = rOther.mPlayInfoList->getInfoNum();
    AudioInfoList<OceanWavePlayInfoInAction>* list = new AudioInfoList<OceanWavePlayInfoInAction>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<OceanWavePlayInfoInAction>;
    list->mInfos->allocBuffer(num + 1, nullptr);
    mPlayInfoList = list;
    for (s32 i = 0; i < num; i++) {
        OceanWavePlayInfoInAction* info =
            rOther.mPlayInfoList != nullptr ? rOther.mPlayInfoList->getInfo(i) : nullptr;
        if (info != nullptr) {
            OceanWavePlayInfoInAction* copy = new OceanWavePlayInfoInAction(*info);
            mPlayInfoList->mInfos->pushBack(copy);
        }
    }
}

/**
 * Copies ocean wave action information.
 * @param rOther information to copy
 * @return this information
 */
OceanWaveActionInfo& OceanWaveActionInfo::operator=(const OceanWaveActionInfo& rOther) {
    mName = rOther.mName;
    if (rOther.mPlayInfoList != nullptr && mPlayInfoList != nullptr) {
        *mPlayInfoList = *rOther.mPlayInfoList;
    }

    return *this;
}

/**
 * Starts the ocean waves of a play information.
 * @param pActor actor that owns the waves
 * @param pName play information name
 */
void startOceanWave(LiveActor* pActor, const char* pName) {
    OceanWaveKeeper* keeper = pActor->mOceanWaveKeeper;
    if (keeper == nullptr || keeper->getUserInfo() == nullptr) {
        return;
    }

    OceanWaveUserInfo* userInfo = keeper->getUserInfo();
    OceanWaveDirector* director = keeper->getDirector();
    if (director == nullptr) {
        return;
    }

    OceanWavePlayInfo* playInfo = nullptr;
    if (pName != nullptr && userInfo->mPlayInfoList != nullptr) {
        playInfo = userInfo->mPlayInfoList->tryFindInfo(pName);
    }

    if (playInfo == nullptr || playInfo->mOceanWaveInfoList == nullptr) {
        return;
    }

    s32 num = playInfo->mOceanWaveInfoList->getInfoNum();
    for (s32 i = 0; i < num; i++) {
        OceanWaveInfo* info = playInfo->mOceanWaveInfoList != nullptr
                                  ? playInfo->mOceanWaveInfoList->getInfo(i)
                                  : nullptr;
        director->createWave(pActor, info);
    }
}

/**
 * Constructs a sphere with default distances.
 */
ActorAlphaCtrl::SphereInfo::SphereInfo() = default;

/**
 * Initializes the sphere from BYAML data.
 * @param rIter BYAML data
 * @param pActor actor that owns the sphere
 */
void ActorAlphaCtrl::SphereInfo::init(const ByamlIter& rIter, LiveActor* pActor) {
    const char* jointName = nullptr;
    tryGetByamlF32(&mNearDist, rIter, "NearDist");
    tryGetByamlF32(&mFarDist, rIter, "FarDist");
    if (tryGetByamlString(&jointName, rIter, "JointName") && jointName != nullptr) {
        if (isEqualString(jointName, "FORCE_ROOT")) {
            mJointMtx = pActor->getBaseMtx();
        } else {
            mJointMtx = getJointMtxPtr(pActor, jointName);
        }
    }

    tryGetByamlV3f(&mPosOffset, rIter);
}

/**
 * Calculates the alpha of the sphere from the camera distance.
 * @param pActor actor that owns the sphere
 * @param pJudge clipping judge that holds the camera position
 * @return alpha
 */
f32 ActorAlphaCtrl::SphereInfo::update(LiveActor* pActor, const ClippingJudge* pJudge) {
    sead::Vector3f pos;
    if (mJointMtx != nullptr) {
        pos.setMul(*mJointMtx, mPosOffset);
    } else {
        pos = getTrans(pActor) + mPosOffset;
    }

    f32 distanceSq = (pos - pJudge->mCameraPos).squaredLength();
    if (distanceSq > mFarDist * mFarDist) {
        return 1.0f;
    }

    if (distanceSq < mNearDist * mNearDist) {
        return 0.0f;
    }

    if (mNearDist < mFarDist) {
        return (sead::Mathf::sqrt(distanceSq) - mNearDist) / (mFarDist - mNearDist);
    }

    return 1.0f;
}

/**
 * Creates an alpha controller if the actor has an alpha control file.
 * @param pActor actor to control
 * @param pResource actor resource
 * @param pFileName suffix of the alpha control file
 * @return created alpha controller, or nullptr
 */
ActorAlphaCtrl* ActorAlphaCtrl::tryCreate(LiveActor* pActor, const Resource* pResource,
                                          const char* pFileName) {
    ByamlIter iter;
    if (tryGetActorInitFileIter(&iter, pResource, "InitAlphaCtrl", pFileName)) {
        return new ActorAlphaCtrl(iter, pActor);
    }

    return nullptr;
}

/**
 * Constructs an alpha controller from BYAML data.
 * @param rIter BYAML data
 * @param pActor actor to control
 */
ActorAlphaCtrl::ActorAlphaCtrl(const ByamlIter& rIter, LiveActor* pActor) : mActor(pActor) {
    mSphereInfo.init(rIter, pActor);
    mSphereInfos = nullptr;
    mSphereInfoNum = 0;
    ByamlIter arrayIter;
    if (!tryGetByamlIterByKey(&arrayIter, rIter, "AlphaCtrlInfoArray") ||
        !arrayIter.isTypeArray()) {
        return;
    }

    mSphereInfoNum = arrayIter.getSize();
    mSphereInfos = new SphereInfo[mSphereInfoNum];
    for (s32 i = 0; i < mSphereInfoNum; i++) {
        ByamlIter iter;
        arrayIter.tryGetIterByIndex(&iter, i);
        mSphereInfos[i].init(iter, mActor);
    }
}

/**
 * Updates the alpha from the camera distance.
 * @param pJudge clipping judge that holds the camera position
 * @return alpha, or 1.0 if the control is off
 */
f32 ActorAlphaCtrl::update(const ClippingJudge* pJudge) {
    if (mSphereInfos == nullptr) {
        mAlpha = mSphereInfo.update(mActor, pJudge);
    } else {
        sead::Vector3f pos;
        if (mSphereInfo.mJointMtx != nullptr) {
            pos.setMul(*mSphereInfo.mJointMtx, mSphereInfo.mPosOffset);
        } else {
            pos = getTrans(mActor) + mSphereInfo.mPosOffset;
        }

        f32 distanceSq = (pos - pJudge->mCameraPos).squaredLength();
        mAlpha = 1.0f;
        if (distanceSq < mSphereInfo.mFarDist * mSphereInfo.mFarDist) {
            for (s32 i = 0; i < mSphereInfoNum; i++) {
                f32 alpha = mSphereInfos[i].update(mActor, pJudge);
                if (alpha < mAlpha) {
                    mAlpha = alpha;
                }
            }
        }
    }

    return mIsOn ? mAlpha : 1.0f;
}
}  // namespace al
