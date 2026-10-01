#include "Project/Camera/Area/CameraAngleVerticalRequester.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {

CameraAngleVerticalRequester::CameraAngleVerticalRequester() = default;

void CameraAngleVerticalRequester::init(AreaObjDirector* pAreaObjDirector) {
    mAreaObjDirector = pAreaObjDirector;
}

void CameraAngleVerticalRequester::initAfterPlacement() {
    mRequestAreaGroup = tryFindAreaObjGroup(this, "CameraAngleVerticalRequestArea");
}

void CameraAngleVerticalRequester::update(const sead::Vector3f& rPos) {
    if (mRequestAreaGroup == nullptr) {
        return;
    }

    AreaObj* area = tryGetAreaObj(mRequestAreaGroup, rPos);

    if (area != mRequestArea) {
        mRequestArea = area;
        mFramesUnchanged = -1;

        if (area != nullptr) {
            getArg(&mAngleVertical, area->getPlacementInfo(), "AngleVertical");
        }
    }

    mFramesUnchanged++;
}

}  // namespace al
