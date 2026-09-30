#include "Library/Model/Function/alModelFunction.hpp"

#include <math/seadMathCalcCommon.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResModel.h>
#include <nn/util/util_VectorApi.h>

#include "Library/Model/alModelCafe.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace alModelFunction {

/**
 * Calculates the radius of a sphere around the model origin containing the model bounding sphere.
 * @param pModel Model.
 * @return Bounding radius.
 */
f32 calcBoundingSphere(const alModelCafe* pModel) {
    const nn::g3d::Sphere* sphere = pModel->getModelG3D()->getModelObj()->GetBounding();
    const sead::Matrix34f& mtx = *pModel->mBaseMtx;
    nn::util::Vector3fType center = sphere->center;
    sead::Vector3f centerPos = {nn::util::VectorGetX(center), nn::util::VectorGetY(center),
                                nn::util::VectorGetZ(center)};
    sead::Vector3f trans = {mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]};
    return (trans - centerPos).length() + sphere->radius;
}

/**
 * Calculates the bounding box enclosing the bounding boxes of all model shapes.
 * @param pBox Output bounding box.
 * @param pModel Model.
 */
void calcBoundingBox(sead::BoundBox3f* pBox, const alModelCafe* pModel) {
    const nn::g3d::ResModel* resModel = pModel->getResModel();
    sead::Vector3f min = {sead::Mathf::maxNumber(), sead::Mathf::maxNumber(),
                          sead::Mathf::maxNumber()};
    sead::Vector3f max = {-sead::Mathf::maxNumber(), -sead::Mathf::maxNumber(),
                          -sead::Mathf::maxNumber()};
    for (s32 i = 0; i < resModel->GetShapeCount(); i++) {
        const nn::g3d::ResShape* shape = resModel->GetShape(i);
        const nn::g3d::Bounding& bounding =
            shape->GetBoundingArray()[shape->GetMesh()->GetSubMeshCount()];
        sead::BoundBox3f box(bounding.center.x - bounding.extent.x,
                             bounding.center.y - bounding.extent.y,
                             bounding.center.z - bounding.extent.z,
                             bounding.center.x + bounding.extent.x,
                             bounding.center.y + bounding.extent.y,
                             bounding.center.z + bounding.extent.z);
        if (box.getMin().x < min.x) {
            min.x = box.getMin().x;
        }
        if (box.getMin().y < min.y) {
            min.y = box.getMin().y;
        }
        if (box.getMin().z < min.z) {
            min.z = box.getMin().z;
        }
        if (box.getMax().x > max.x) {
            max.x = box.getMax().x;
        }
        if (box.getMax().y > max.y) {
            max.y = box.getMax().y;
        }
        if (box.getMax().z > max.z) {
            max.z = box.getMax().z;
        }
    }
    pBox->set(min, max);
}

}  // namespace alModelFunction
