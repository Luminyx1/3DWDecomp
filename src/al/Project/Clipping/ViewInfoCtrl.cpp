#include "Project/Clipping/ViewInfoCtrl.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Clipping/ClippingActorInfo.hpp"

namespace al {
    s32 getPlayerNumMax(const PlayerHolder* pHolder);
    bool isPlayerDead(const PlayerHolder* pHolder, s32 index);
    const sead::Vector3f& getPlayerPos(const PlayerHolder* pHolder, s32 index);

    /**
     * @brief Constructs the controller with the default view info.
     * @param pAreaObjDirector The scene's area director.
     * @param pPlayerHolder The scene's player holder.
     */
    ViewInfoCtrl::ViewInfoCtrl(const AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder)
        : mAreaObjDirector(pAreaObjDirector), mViewCtrlAreaGroup(nullptr), mDefaultViewInfo(nullptr), mViewInfoNum(0),
          mViewInfos(nullptr), mIsInvalid(false), mPlayerHolder(pPlayerHolder) {
        mViewInfos = new ViewInfo*[128];
        for (s32 i = 0; i < 128; i++) {
            mViewInfos[i] = nullptr;
        }

        mDefaultViewInfo = new ViewInfo();
        mViewInfos[0] = new ViewInfo();
        mViewInfoNum = 1;
        mViewInfos[0]->mPlacementId = nullptr;
        mViewInfos[0]->mIsInViewCtrlArea = true;
    }

    /**
     * @brief Links an actor's clipping info to the view info of its view group.
     * @param pActorInfo The actor's clipping info.
     * @param pViewId The actor's view group id, or nullptr.
     */
    void ViewInfoCtrl::initActorInfo(ClippingActorInfo* pActorInfo, PlacementId* pViewId) {
        if (pViewId == nullptr || pViewId->mPlacementID == nullptr) {
            pActorInfo->mViewGroupFarClipFlag = &mDefaultViewInfo->mIsInViewCtrlArea;
            return;
        }

        for (s32 i = 0; i < mViewInfoNum; i++) {
            ViewInfo* viewInfo = mViewInfos[i];
            if (viewInfo->mPlacementId != nullptr && viewInfo->mPlacementId->isEqual(*pViewId)) {
                pActorInfo->mViewGroupFarClipFlag = &viewInfo->mIsInViewCtrlArea;
                return;
            }
        }

        ViewInfo* newViewInfo = new ViewInfo();
        newViewInfo->mPlacementId = pViewId;
        pActorInfo->mViewGroupFarClipFlag = &newViewInfo->mIsInViewCtrlArea;
        mViewInfos[mViewInfoNum] = newViewInfo;
        mViewInfoNum++;
    }

    /** @brief Looks up the ViewCtrlArea group once all areas are registered. */
    void ViewInfoCtrl::endInit() {
        mViewCtrlAreaGroup = mAreaObjDirector->getAreaObjGroup("ViewCtrlArea");
    }

    /** @brief Marks the view groups of the ViewCtrlAreas that contain a player. */
    void ViewInfoCtrl::update() {
        if (mIsInvalid) {
            return;
        }

        if (mViewCtrlAreaGroup == nullptr) {
            return;
        }

        for (s32 i = 0; i < mViewInfoNum; i++) {
            ViewInfo* viewInfo = mViewInfos[i];
            viewInfo->mIsInViewCtrlArea = false;
            viewInfo->_9 = false;
        }

        for (s32 i = 0; i < mViewCtrlAreaGroup->getAreaObjCount(); i++) {
            AreaObj* areaObj = mViewCtrlAreaGroup->getAreaObj(i);
            s32 playerNumMax = getPlayerNumMax(mPlayerHolder);
            for (s32 j = 0; j < playerNumMax; j++) {
                if (isPlayerDead(mPlayerHolder, j)) {
                    continue;
                }

                if (tryIsInAreaPos(areaObj, getPlayerPos(mPlayerHolder, j))) {
                    PlacementId viewId;
                    alPlacementFunction::getClippingViewId(&viewId, *areaObj->mPlacementInfo);
                    ViewInfo* viewInfo = tryFindViewInfo(&viewId);
                    if (viewInfo != nullptr) {
                        viewInfo->mIsInViewCtrlArea = true;
                    }
                    break;
                }
            }
        }
    }

    /**
     * @brief Searches for the view info of a view group.
     * @param pViewId The view group id.
     * @return The view info, or nullptr if there is none.
     */
    ViewInfoCtrl::ViewInfo* ViewInfoCtrl::tryFindViewInfo(PlacementId* pViewId) const {
        if (pViewId == nullptr) {
            return nullptr;
        }

        for (s32 i = 0; i < mViewInfoNum; i++) {
            ViewInfo* viewInfo = mViewInfos[i];
            if (viewInfo->mPlacementId != nullptr && viewInfo->mPlacementId->isEqual(*pViewId)) {
                return viewInfo;
            }
        }

        return nullptr;
    }
};
