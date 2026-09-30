#include "Project/LiveActor/ConveyerKeyKeeper.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"

namespace {
using namespace al;

/**
 * Initializes a conveyer key from its placement.
 * @param pKey The key to initialize.
 * @param rPlacementInfo The placement of the key.
 * @param rKeeperTrans The translation of the keeper.
 * @param rKeeperDir The move direction of the keeper.
 */
void initConveyerKey(ConveyerKey* pKey, const PlacementInfo& rPlacementInfo,
                     const sead::Vector3f& rKeeperTrans, const sead::Vector3f& rKeeperDir) {
    sead::Vector3f trans;
    tryGetTrans(&trans, rPlacementInfo);
    pKey->mMoveDistance = (trans - rKeeperTrans).dot(rKeeperDir);
    pKey->mTotalMoveDistance = 0.0f;
    pKey->mInterpolateType = 0;
    tryGetArg(&pKey->mInterpolateType, rPlacementInfo, "InterpolateType");
    static_cast<sead::BaseQuat<f32>&>(pKey->mQuat) = sead::Quatf::unit;
    pKey->mPlacementInfo = new PlacementInfo(rPlacementInfo);
    tryGetQuat(&pKey->mQuat, rPlacementInfo);
    verticalizeVec(&pKey->mMoveDistanceVertical, rKeeperDir, trans - rKeeperTrans);
}
}  // namespace

namespace al {
/**
 * Constructs an empty conveyer key keeper.
 */
ConveyerKeyKeeper::ConveyerKeyKeeper() = default;

/**
 * Reads the keys linked to an actor.
 * @param rInfo The actor init info.
 */
void ConveyerKeyKeeper::init(const ActorInitInfo& rInfo) {
    tryGetQuat(&mQuat, rInfo);
    tryGetTrans(&mTrans, rInfo);
    s32 moveAxis = 2;
    tryGetArg(&moveAxis, rInfo, "MoveAxis");
    tryGetLocalAxis(&mMoveDirection, rInfo, moveAxis);
    mConveyerKeyCount = calcLinkNestNum(rInfo, "KeyMoveNext") + 1;
    mConveyerKeys = new ConveyerKey[mConveyerKeyCount];
    initConveyerKey(&mConveyerKeys[0], *rInfo.mPlacementInfo, mTrans, mMoveDirection);

    PlacementInfo linkPlacementSource = *rInfo.mPlacementInfo;
    PlacementInfo linkPlacement;

    for (s32 i = 0; i < mConveyerKeyCount - 1; i++) {
        getLinksInfo(&linkPlacement, linkPlacementSource, "KeyMoveNext");
        initConveyerKey(&mConveyerKeys[i + 1], linkPlacement, mTrans, mMoveDirection);
        linkPlacementSource = linkPlacement;
    }

    mTotalMoveDistance = 0.0f;

    for (s32 i = 1; i < mConveyerKeyCount; i++) {
        ConveyerKey* key = &mConveyerKeys[i - 1];
        ConveyerKey* nextKey = &mConveyerKeys[i];
        mTotalMoveDistance += sead::Mathf::abs(nextKey->mMoveDistance - key->mMoveDistance);
        nextKey->mTotalMoveDistance = mTotalMoveDistance;
    }
}

/**
 * Calculates the position and rotation at a distance along the keys.
 * @param pPos The position, or nullptr.
 * @param pQuat The rotation, or nullptr.
 * @param pIndex The index of the key before the position, or nullptr.
 * @param coord The distance along the keys.
 */
void ConveyerKeyKeeper::calcPosAndQuat(sead::Vector3f* pPos, sead::Quatf* pQuat, s32* pIndex,
                                       f32 coord) const {
    if (coord <= 0.0f) {
        if (pPos) {
            pPos->set(mTrans);
        }

        if (pQuat) {
            pQuat->set(mQuat);
        }

        if (pIndex) {
            *pIndex = -1;
        }

        return;
    }

    if (coord >= mTotalMoveDistance) {
        if (pPos) {
            const ConveyerKey& key = getConveyerKey(mConveyerKeyCount - 1);
            pPos->set(key.mMoveDistance * mMoveDirection + mTrans + key.mMoveDistanceVertical);
        }

        if (pQuat) {
            pQuat->set(getConveyerKey(mConveyerKeyCount - 1).mQuat);
        }

        if (pIndex) {
            *pIndex = -1;
        }

        return;
    }

    s32 keyIndex = 0;

    for (s32 i = 0; i < mConveyerKeyCount; i++) {
        if (getConveyerKey(i).mTotalMoveDistance > coord) {
            keyIndex = i;
            break;
        }
    }

    sead::Vector3f moveDistanceVertical = sead::Vector3f::zero;
    sead::Quatf quat = sead::Quatf::unit;
    f32 moveDistance;

    if (keyIndex < 1) {
        const ConveyerKey& key = getConveyerKey(0);
        moveDistanceVertical.set(key.mMoveDistanceVertical);
        quat.set(key.mQuat);
        moveDistance = 0.0f;
    } else {
        const ConveyerKey& key = getConveyerKey(keyIndex);
        const ConveyerKey& prevKey = getConveyerKey(keyIndex - 1);
        sead::Vector3f prevKeyVec = prevKey.mMoveDistanceVertical;
        sead::Vector3f keyVec = key.mMoveDistanceVertical;
        f32 totalMoveDistance = key.mTotalMoveDistance - prevKey.mTotalMoveDistance;
        f32 rate;

        if (isNearZero(totalMoveDistance)) {
            rate = 0.0f;
        } else {
            rate = (coord - getConveyerKey(keyIndex - 1).mTotalMoveDistance) / totalMoveDistance;
        }

        f32 ease = easeByType(rate, getConveyerKey(keyIndex - 1).mInterpolateType);
        lerpVec(&moveDistanceVertical, prevKeyVec, keyVec, ease);
        moveDistance = lerpValue(rate, getConveyerKey(keyIndex - 1).mMoveDistance,
                                 getConveyerKey(keyIndex).mMoveDistance);
        sead::Quatf prevKeyQuat = getConveyerKey(keyIndex - 1).mQuat;
        sead::Quatf keyQuat = getConveyerKey(keyIndex).mQuat;
        slerpQuat(&quat, prevKeyQuat, keyQuat, ease);
    }

    if (pPos) {
        pPos->set(moveDistance * mMoveDirection + mTrans + moveDistanceVertical);
    }

    if (pQuat) {
        pQuat->set(quat);
    }

    if (pIndex) {
        *pIndex = keyIndex - 1;
    }
}

/**
 * Calculates the position and rotation of a key.
 * @param pPos The position, or nullptr.
 * @param pQuat The rotation, or nullptr.
 * @param index The key index.
 */
void ConveyerKeyKeeper::calcPosAndQuatByKeyIndex(sead::Vector3f* pPos, sead::Quatf* pQuat,
                                                 s32 index) const {
    const ConveyerKey& key = getConveyerKey(index);

    if (pPos) {
        pPos->set(key.mMoveDistance * mMoveDirection + mTrans + key.mMoveDistanceVertical);
    }

    if (pQuat) {
        pQuat->set(getConveyerKey(index).mQuat);
    }
}

/**
 * Calculates a sphere containing all keys.
 * @param pTrans The sphere center.
 * @param pRadius The sphere radius.
 * @param offset The radius around each key.
 */
void ConveyerKeyKeeper::calcClippingSphere(sead::Vector3f* pTrans, f32* pRadius,
                                           f32 offset) const {
    if (pTrans) {
        const ConveyerKey& key = getConveyerKey(0);
        pTrans->set(key.mMoveDistance * mMoveDirection + mTrans + key.mMoveDistanceVertical);
    }

    *pRadius = offset;
    for (s32 i = 1; i < mConveyerKeyCount; i++) {
        const ConveyerKey& key = getConveyerKey(i);
        sead::Vector3f pos =
            key.mMoveDistance * mMoveDirection + mTrans + key.mMoveDistanceVertical;
        calcSphereMargeSpheres(pTrans, pRadius, *pTrans, *pRadius, pos, offset);
    }
}

/**
 * Gets a key.
 * @param index The key index.
 * @return The key.
 */
const ConveyerKey& ConveyerKeyKeeper::getConveyerKey(s32 index) const {
    return mConveyerKeys[index];
}
}  // namespace al
