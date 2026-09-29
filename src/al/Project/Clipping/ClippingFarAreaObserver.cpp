#include "Project/Clipping/ClippingFarAreaObserver.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
    s32 getPlayerNumMax(const PlayerHolder* pHolder);
    bool isPlayerDead(const PlayerHolder* pHolder, s32 index);
    const sead::Vector3f& getPlayerPos(const PlayerHolder* pHolder, s32 index);

    /**
     * @brief Constructs the observer with the default far clip distances.
     * @param pAreaObjDirector The scene's area director.
     * @param pPlayerHolder The scene's player holder.
     */
    ClippingFarAreaObserver::ClippingFarAreaObserver(const AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder)
        : mAreaObjDirector(pAreaObjDirector), mPlayerHolder(pPlayerHolder), mAreaObjGroup(nullptr), mAreaObj(nullptr),
          mFarClipDistance(7000.0f), mDefaultFarClipDistance(7000.0f), mFarClipDistanceSub(4000.0f),
          mDefaultFarClipDistanceSub(4000.0f), mFarClipDistanceRate(1.0f) {}

    /**
     * @brief Sets the far clip distance used outside of any ClippingFarArea.
     * @param distance The far clip distance.
     */
    void ClippingFarAreaObserver::setDefaultFarClipDistance(f32 distance) {
        mFarClipDistance = distance;
        mDefaultFarClipDistance = distance;
    }

    /**
     * @brief Sets the sub far clip distance used outside of any ClippingFarArea.
     * @param distance The sub far clip distance.
     */
    void ClippingFarAreaObserver::setDefaultFarClipDistanceSub(f32 distance) {
        mFarClipDistanceSub = distance;
        mDefaultFarClipDistanceSub = distance;
    }

    /** @brief Looks up the ClippingFarArea group once all areas are registered. */
    void ClippingFarAreaObserver::endInit() {
        mAreaObjGroup = mAreaObjDirector->getAreaObjGroup("ClippingFarArea");
    }

    /** @brief Finds the highest priority ClippingFarArea a living player is in and applies its distances. */
    void ClippingFarAreaObserver::update() {
        if (mAreaObjGroup == nullptr) {
            return;
        }

        mAreaObj = nullptr;
        s32 playerNumMax = getPlayerNumMax(mPlayerHolder);
        for (s32 i = 0; i < playerNumMax; i++) {
            if (isPlayerDead(mPlayerHolder, i)) {
                continue;
            }

            AreaObj* areaObj = mAreaObjGroup->getInVolumeAreaObj(getPlayerPos(mPlayerHolder, i));
            if (areaObj != nullptr && (mAreaObj == nullptr || areaObj->mPriority > mAreaObj->mPriority)) {
                mAreaObj = areaObj;
            }
        }

        mFarClipDistance = mDefaultFarClipDistance;
        mFarClipDistanceSub = mDefaultFarClipDistanceSub;
        if (mAreaObj != nullptr) {
            tryGetAreaObjArg(&mFarClipDistance, mAreaObj, "FarClipDistance");
            tryGetAreaObjArg(&mFarClipDistanceSub, mAreaObj, "FarClipDistanceSub");
        }
    }
};
