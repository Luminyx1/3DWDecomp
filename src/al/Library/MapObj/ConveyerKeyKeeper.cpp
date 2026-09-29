#include "Library/MapObj/ConveyerKeyKeeper.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Play/Placement/PlacementUtil.hpp"

namespace {
    using namespace al;

    /**
     * @brief Initializes a conveyer key from its placement.
     * @param pKey The key to initialize.
     * @param rPlacement The placement info of the key.
     * @param rKeeperTrans The position of the conveyer.
     * @param rKeeperDir The move direction of the conveyer.
     */
    void initConveyerKey(ConveyerKey* pKey, const PlacementInfo& rPlacement, const sead::Vector3f& rKeeperTrans,
                         const sead::Vector3f& rKeeperDir) {
        sead::Vector3f trans;
        tryGetTrans(&trans, rPlacement);
        pKey->mMoveDistance = (trans - rKeeperTrans).dot(rKeeperDir);
        pKey->mTotalMoveDistance = 0.0f;

        pKey->mInterpolateType = 0;
        tryGetArg(&pKey->mInterpolateType, rPlacement, "InterpolateType");

        static_cast<sead::BaseQuat<f32>&>(pKey->mQuat) = sead::Quatf::unit;
        pKey->mPlacementInfo = new PlacementInfo(rPlacement);
        tryGetQuat(&pKey->mQuat, rPlacement);
        verticalizeVec(&pKey->mMoveDistanceVertical, rKeeperDir, trans - rKeeperTrans);
    }
}  // namespace

namespace al {
    /**
     * @brief Constructs an empty conveyer key keeper.
     */
    ConveyerKeyKeeper::ConveyerKeyKeeper() = default;

    /**
     * @brief Reads the conveyer pose, move axis and all keys linked with KeyMoveNext.
     * @param rInfo The actor init info of the conveyer.
     */
    void ConveyerKeyKeeper::init(const ActorInitInfo& rInfo) {
        tryGetQuat(&mQuat, rInfo);
        tryGetTrans(&mTrans, rInfo);
        s32 moveAxis = 2;
        tryGetArg(&moveAxis, rInfo, "MoveAxis");
        tryGetLocalAxis(&mMoveDirection, rInfo, moveAxis);
        mConveyerKeyNum = calcLinkNestNum(rInfo, "KeyMoveNext") + 1;
        mConveyerKeys = new ConveyerKey[mConveyerKeyNum];

        initConveyerKey(&mConveyerKeys[0], *rInfo.mPlacementInfo, mTrans, mMoveDirection);

        PlacementInfo linkSource(*rInfo.mPlacementInfo);
        PlacementInfo link;
        for (s32 i = 0; i < mConveyerKeyNum - 1; i++) {
            getLinksInfo(&link, linkSource, "KeyMoveNext");
            initConveyerKey(&mConveyerKeys[i + 1], link, mTrans, mMoveDirection);
            linkSource = link;
        }

        mTotalMoveDistance = 0.0f;
        for (s32 i = 1; i < mConveyerKeyNum; i++) {
            ConveyerKey* prevKey = &mConveyerKeys[i - 1];
            ConveyerKey* key = &mConveyerKeys[i];
            mTotalMoveDistance += sead::Mathf::abs(key->mMoveDistance - prevKey->mMoveDistance);
            key->mTotalMoveDistance = mTotalMoveDistance;
        }
    }

    /**
     * @brief Calculates the pose at a distance along the conveyer.
     * @param pPos Output for the position, may be nullptr.
     * @param pQuat Output for the rotation, may be nullptr.
     * @param pIndex Output for the index of the key segment, -1 when outside of the conveyer, may be nullptr.
     * @param coord The distance along the conveyer.
     */
    void ConveyerKeyKeeper::calcPosAndQuat(sead::Vector3f* pPos, sead::Quatf* pQuat, s32* pIndex, f32 coord) const {
        if (coord <= 0.0f) {
            if (pPos != nullptr) {
                static_cast<sead::BaseVec3<f32>&>(*pPos) = mTrans;
            }
            if (pQuat != nullptr) {
                static_cast<sead::BaseQuat<f32>&>(*pQuat) = mQuat;
            }
            if (pIndex != nullptr) {
                *pIndex = -1;
            }
            return;
        }

        if (mTotalMoveDistance <= coord) {
            if (pPos != nullptr) {
                const ConveyerKey& key = getConveyerKey(mConveyerKeyNum - 1);
                pPos->set((key.mMoveDistance * mMoveDirection + mTrans) + key.mMoveDistanceVertical);
            }
            if (pQuat != nullptr) {
                static_cast<sead::BaseQuat<f32>&>(*pQuat) = getConveyerKey(mConveyerKeyNum - 1).mQuat;
            }
            if (pIndex != nullptr) {
                *pIndex = -1;
            }
            return;
        }

        s32 keyIndex = 0;
        for (s32 i = 0; i < mConveyerKeyNum; i++) {
            if (getConveyerKey(i).mTotalMoveDistance > coord) {
                keyIndex = i;
                break;
            }
        }

        sead::Vector3f moveDistanceVertical = sead::Vector3f::zero;
        sead::Quatf quat = sead::Quatf::unit;
        f32 moveDistance;
        if (keyIndex == 0) {
            const ConveyerKey& key = getConveyerKey(0);
            static_cast<sead::BaseVec3<f32>&>(moveDistanceVertical) = key.mMoveDistanceVertical;
            static_cast<sead::BaseQuat<f32>&>(quat) = key.mQuat;
            moveDistance = 0.0f;
        } else {
            const ConveyerKey& prevKey = mConveyerKeys[keyIndex - 1];
            const ConveyerKey& key = mConveyerKeys[keyIndex];
            sead::Vector3f prevVertical = prevKey.mMoveDistanceVertical;
            sead::Vector3f vertical = key.mMoveDistanceVertical;
            f32 distance = key.mTotalMoveDistance - prevKey.mTotalMoveDistance;
            f32 rate;
            if (isNearZero(distance, 0.001f)) {
                rate = 0.0f;
            } else {
                rate = (coord - mConveyerKeys[keyIndex - 1].mTotalMoveDistance) / distance;
            }

            f32 easeRate = easeByType(rate, mConveyerKeys[keyIndex - 1].mInterpolateType);
            lerpVec(&moveDistanceVertical, prevVertical, vertical, easeRate);
            moveDistance = lerpValue(rate, mConveyerKeys[keyIndex - 1].mMoveDistance,
                                     mConveyerKeys[keyIndex].mMoveDistance);

            sead::Quatf prevQuat = mConveyerKeys[keyIndex - 1].mQuat;
            sead::Quatf nextQuat = mConveyerKeys[keyIndex].mQuat;
            slerpQuat(&quat, prevQuat, nextQuat, easeRate);
        }

        if (pPos != nullptr) {
            pPos->set(moveDistance * mMoveDirection + mTrans + moveDistanceVertical);
        }
        if (pQuat != nullptr) {
            static_cast<sead::BaseQuat<f32>&>(*pQuat) = quat;
        }
        if (pIndex != nullptr) {
            *pIndex = keyIndex - 1;
        }
    }

    /**
     * @brief Calculates the pose of a key.
     * @param pPos Output for the position, may be nullptr.
     * @param pQuat Output for the rotation, may be nullptr.
     * @param index The key index.
     */
    void ConveyerKeyKeeper::calcPosAndQuatByKeyIndex(sead::Vector3f* pPos, sead::Quatf* pQuat, s32 index) const {
        const ConveyerKey& key = getConveyerKey(index);
        if (pPos != nullptr) {
            pPos->set((key.mMoveDistance * mMoveDirection + mTrans) + key.mMoveDistanceVertical);
        }
        if (pQuat != nullptr) {
            static_cast<sead::BaseQuat<f32>&>(*pQuat) = getConveyerKey(index).mQuat;
        }
    }

    /**
     * @brief Calculates a sphere containing every key.
     * @param pClippingTrans Output for the sphere center.
     * @param pClippingRadius Output for the sphere radius.
     * @param offset The radius around each key.
     */
    void ConveyerKeyKeeper::calcClippingSphere(sead::Vector3f* pClippingTrans, f32* pClippingRadius,
                                               f32 offset) const {
        if (pClippingTrans != nullptr) {
            const ConveyerKey& key = mConveyerKeys[0];
            pClippingTrans->set((key.mMoveDistance * mMoveDirection + mTrans) + key.mMoveDistanceVertical);
        }
        *pClippingRadius = offset;

        for (s32 i = 1; i < mConveyerKeyNum; i++) {
            const ConveyerKey& key = mConveyerKeys[i];
            sead::Vector3f pos = (key.mMoveDistance * mMoveDirection + mTrans) + key.mMoveDistanceVertical;
            calcSphereMargeSpheres(pClippingTrans, pClippingRadius, *pClippingTrans, *pClippingRadius, pos, offset);
        }
    }

    /**
     * @brief Gets a key.
     * @param index The key index.
     * @return The key.
     */
    const ConveyerKey& ConveyerKeyKeeper::getConveyerKey(s32 index) const {
        return mConveyerKeys[index];
    }
}  // namespace al
