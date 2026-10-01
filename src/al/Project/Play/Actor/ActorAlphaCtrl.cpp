#include "Project/Play/Actor/ActorAlphaCtrl.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
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
    sead::Vector3f pos = calcPos(pActor);
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
