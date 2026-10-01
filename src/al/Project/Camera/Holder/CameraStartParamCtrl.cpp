#include "Project/Camera/Holder/CameraStartParamCtrl.hpp"

#include "Library/Camera/CameraStartInfo.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Camera/Area/CameraStartParamArea.hpp"

namespace al {

/**
 * Creates the control without areas.
 */
CameraStartParamCtrl::CameraStartParamCtrl() = default;

/**
 * Sets the area director and camera flags.
 * @param pDirector Area object director.
 * @param pFlagCtrl Camera flags.
 */
void CameraStartParamCtrl::init(AreaObjDirector* pDirector, const CameraFlagCtrl* pFlagCtrl) {
    mAreaObjDirector = pDirector;
    mFlagCtrl = pFlagCtrl;
}

/**
 * Finds the camera start parameter areas.
 */
void CameraStartParamCtrl::initAfterPlacement() {
    mAreaGroup = tryFindAreaObjGroup(this, "CameraStartParamArea");
    mAreaGroupKids = tryFindAreaObjGroup(this, "CameraStartParamAreaKids");
}

static void updateCurrentArea(CameraStartParamArea** pCurrentArea, AreaObjGroup* pGroup,
                              const sead::Vector3f& rPos) {
    for (s32 i = 0; i < pGroup->getSize(); i++) {
        CameraStartParamArea* area = static_cast<CameraStartParamArea*>(pGroup->getAreaObj(i));

        if (*pCurrentArea != nullptr && area->getPriority() < (*pCurrentArea)->mPriority) {
            continue;
        }

        if (area->isValidParam() && area->isInVolume(rPos)) {
            *pCurrentArea = area;
        }
    }
}

/**
 * Finds the start parameter area with the highest priority at a position.
 * @param rPos Position to check.
 */
void CameraStartParamCtrl::update(const sead::Vector3f& rPos) {
    mCurrentArea = nullptr;

    if (mAreaGroup != nullptr) {
        updateCurrentArea(&mCurrentArea, mAreaGroup, rPos);
    }

    if (alCameraFunction::isValidCameraAreaKids(mFlagCtrl) && mAreaGroupKids != nullptr) {
        updateCurrentArea(&mCurrentArea, mAreaGroupKids, rPos);
    }
}

/**
 * Applies the angles of the current start parameter area to a camera start info.
 * @param pInfo Camera start info.
 */
void CameraStartParamCtrl::tryApplyParam(CameraStartInfo* pInfo) {
    if (mCurrentArea == nullptr) {
        return;
    }

    if (mCurrentArea->getAngleH()) {
        pInfo->setAreaAngleH(*mCurrentArea->getAngleH());
    }

    if (mCurrentArea->getAngleV()) {
        pInfo->setAreaAngleV(*mCurrentArea->getAngleV());
    }

    if (mCurrentArea->isOneTime()) {
        mCurrentArea->invalidateParam();
    }
}

}  // namespace al
