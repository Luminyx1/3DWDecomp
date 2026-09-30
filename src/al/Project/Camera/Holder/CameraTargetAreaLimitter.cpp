#include "Project/Camera/Holder/CameraTargetAreaLimitter.hpp"

#include <math/seadMatrix.h>

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/AreaObj/AreaShape.hpp"
#include "Project/AreaObj/AreaShapeFactory.hpp"

namespace al {

/**
 * Creates a limitter from the target limit area linked to a placement.
 * @param rInfo Placement info.
 * @return New limitter, or null if no area is linked.
 */
CameraTargetAreaLimitter* CameraTargetAreaLimitter::tryCreate(const PlacementInfo& rInfo) {
    if (!isExistLinkChild(rInfo, "TargetLimitArea", 0)) {
        return nullptr;
    }

    PlacementInfo linkInfo;
    getLinksInfo(&linkInfo, rInfo, "TargetLimitArea");
    const char* modelName = nullptr;
    alPlacementFunction::getModelName(&modelName, linkInfo);

    AreaShapeFactory factory("エリアシェイプファクトリー");
    AreaShapeCreatorFunction creator = nullptr;
    factory.getEntryIndex(&creator, modelName);

    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    tryGetScale(&scale, linkInfo);
    AreaShape* shape = creator();
    shape->setScale(scale);
    sead::Matrix34f* baseMtx = new sead::Matrix34f(sead::Matrix34f::ident);
    tryGetMatrixTR(baseMtx, linkInfo);
    shape->setBaseMtxPtr(baseMtx);
    return new CameraTargetAreaLimitter(shape);
}

/**
 * Creates a limitter for an area shape.
 * @param pShape Area shape the target is limited to.
 */
CameraTargetAreaLimitter::CameraTargetAreaLimitter(const AreaShape* pShape) : mAreaShape(pShape) {}

/**
 * Moves a position outside of the area onto the nearest edge of the area.
 * @param pOut Receives the limited position.
 * @param rPos Position to limit.
 * @return Whether the position was outside of the area.
 */
bool CameraTargetAreaLimitter::applyAreaLimit(sead::Vector3f* pOut,
                                              const sead::Vector3f& rPos) const {
    if (mAreaShape->isInVolume(rPos)) {
        return false;
    }

    mAreaShape->calcNearestEdgePoint(pOut, rPos);
    return true;
}

}  // namespace al
