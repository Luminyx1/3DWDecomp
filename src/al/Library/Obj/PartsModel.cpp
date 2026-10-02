#include "Library/Obj/PartsModel.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Obj/PartsFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs a model following a joint of a parent.
 * @param pName actor name
 */
PartsModel::PartsModel(const char* pName) : LiveActor(pName) {}

/**
 * Follows the given matrix.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pArchiveName model archive name
 * @param pJointMtx matrix to follow
 * @param isUseFollowMtxScale whether the scale of the matrix is used
 */
void PartsModel::initPartsMtx(LiveActor* pParent, const ActorInitInfo& rInfo,
                              const char* pArchiveName, const sead::Matrix34f* pJointMtx,
                              bool isUseFollowMtxScale) {
    mParentModel = pParent;
    mJointMtx = pJointMtx;
    mIsUseFollowMtxScale = isUseFollowMtxScale;
    initActorWithArchiveNameNoPlacementInfo(this, rInfo, pArchiveName, nullptr);
    StringTmp<128>("[PartsModel] %s", getName()).cstr();
    invalidateClipping(this);
    makeActorAppeared();
}

/**
 * Follows the given matrix, using a suffixed archive.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pArchiveName model archive name
 * @param pSuffix archive suffix
 * @param pJointMtx matrix to follow
 * @param isUseFollowMtxScale whether the scale of the matrix is used
 */
void PartsModel::initPartsSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                 const char* pArchiveName, const char* pSuffix,
                                 const sead::Matrix34f* pJointMtx, bool isUseFollowMtxScale) {
    mParentModel = pParent;
    mJointMtx = pJointMtx;
    mIsUseFollowMtxScale = isUseFollowMtxScale;
    initActorWithArchiveNameNoPlacementInfo(this, rInfo, pArchiveName, pSuffix);
    StringTmp<128>("[PartsModel] %s", getName()).cstr();
    invalidateClipping(this);
    makeActorAppeared();
}

/**
 * Follows the joint and offset defined in the parent's InitPartsFixInfo.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pArchiveName model archive name
 * @param pArchiveSuffix archive suffix
 * @param pSuffix InitPartsFixInfo suffix
 */
void PartsModel::initPartsFixFile(LiveActor* pParent, const ActorInitInfo& rInfo,
                                  const char* pArchiveName, const char* pArchiveSuffix,
                                  const char* pSuffix) {
    mParentModel = pParent;
    mJointMtx = pParent->getBaseMtx();
    initActorWithArchiveNameNoPlacementInfo(this, rInfo, pArchiveName, pArchiveSuffix);
    invalidateClipping(this);
    StringTmp<128> initFileName;
    createFileNameBySuffix(&initFileName, "InitPartsFixInfo", pSuffix);

    if (isExistModelResourceYaml(mParentModel, initFileName.cstr(), nullptr)) {
        ByamlIter iter(getModelResourceYaml(mParentModel, initFileName.cstr(), nullptr));
        const char* jointName = nullptr;
        iter.tryGetStringByKey(&jointName, "JointName");

        if (jointName != nullptr) {
            mJointMtx = getJointMtxPtr(mParentModel, jointName);
        }

        tryGetByamlV3f(&mLocalTrans, iter, "LocalTrans");
        tryGetByamlV3f(&mLocalRotate, iter, "LocalRotate");
        tryGetByamlV3f(&mLocalScale, iter, "LocalScale");

        if (!isNearZero(mLocalTrans) || !isNearZero(mLocalRotate)) {
            mIsUseLocalPos = true;
        }

        mIsUseFollowMtxScale = tryGetByamlKeyBoolOrFalse(iter, "UseFollowMtxScale");
        mIsUseLocalScale = tryGetByamlKeyBoolOrFalse(iter, "UseLocalScale");
    }

    makeActorAppeared();
}

/**
 * Updates the pose and appears.
 */
void PartsModel::makeActorAppeared() {
    updatePose();
    LiveActor::makeActorAppeared();
    mIsHostHidden = false;
}

/**
 * Updates the pose from the followed matrix and the local offset.
 */
void PartsModel::updatePose() {
    if (!mIsUseLocalPos) {
        sead::Vector3f mtxScale;
        sead::Matrix34f baseMtx = *mJointMtx;

        if (mIsUseFollowMtxScale) {
            calcMtxScale(&mtxScale, baseMtx);
            const sead::Vector3f& scale = sead::Vector3f::ones;
            mtxScale.x = scale.x * mtxScale.x;
            mtxScale.y = scale.y * mtxScale.y;
            mtxScale.z = scale.z * mtxScale.z;
            setScale(this, mtxScale);
        }

        normalize(&baseMtx);
        updatePoseMtx(this, &baseMtx);
        return;
    }

    sead::Matrix34f rotateMtx;
    sead::Vector3f rotate(sead::Mathf::deg2rad(mLocalRotate.x),
                          sead::Mathf::deg2rad(mLocalRotate.y),
                          sead::Mathf::deg2rad(mLocalRotate.z));
    rotateMtx.makeR(rotate);
    sead::Matrix34f transMtx;
    transMtx.makeRT({0.0f, 0.0f, 0.0f}, mLocalTrans);
    sead::Matrix34f poseMtx = rotateMtx * transMtx;
    sead::Matrix34f baseMtx = *mJointMtx;

    if (mIsUseFollowMtxScale) {
        sead::Vector3f mtxScale;
        const sead::Vector3f& scale = mIsUseLocalScale ? mLocalScale : sead::Vector3f::ones;
        calcMtxScale(&mtxScale, baseMtx);
        mtxScale.x = scale.x * mtxScale.x;
        mtxScale.y = scale.y * mtxScale.y;
        mtxScale.z = scale.z * mtxScale.z;
        setScale(this, mtxScale);
    } else if (mIsUseLocalScale) {
        setScale(this, mLocalScale);
    }

    normalize(&baseMtx);
    baseMtx = baseMtx * poseMtx;
    updatePoseMtx(this, &baseMtx);
}

/**
 * Hides with the host and follows it while visible.
 */
void PartsModel::control() {
    if (updateSyncHostVisible(&mIsHostHidden, this, mParentModel, mIsForceHide)) {
        updatePose();
    }
}

/**
 * Hides or shows with the host.
 */
void PartsModel::syncHostVisible() {
    updateSyncHostVisible(&mIsHostHidden, this, mParentModel, mIsForceHide);
}

/**
 * Forwards sensor attacks to the parent while visible.
 * @param pSelf own sensor
 * @param pOther other sensor
 */
void PartsModel::attackSensor(HitSensor* pSelf, HitSensor* pOther) {
    if (mIsHostHidden) {
        return;
    }

    mParentModel->attackSensor(pSelf, pOther);
}

/**
 * Forwards messages to the parent while visible.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the parent handled the message
 */
bool PartsModel::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (mIsHostHidden) {
        return false;
    }

    return mParentModel->receiveMsg(pMsg, pOther, pSelf);
}

/**
 * Keeps following the parent while paused.
 * @param isPaused whether the actor is paused
 */
void PartsModel::movementPaused(bool isPaused) {
    control();
    LiveActor::movementPaused(isPaused);
}
}  // namespace al
