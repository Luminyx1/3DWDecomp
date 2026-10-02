#include "Library/Obj/SkyProjection.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <math/seadMatrix.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_Resources.h>

#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace {
/**
 * Writes a shader parameter of a material if the material has it.
 * @param pMaterial material to edit
 * @param pName shader parameter name
 * @param rValue value to write
 */
template <typename T, typename V>
inline void trySetShaderParam(nn::g3d::MaterialObj* pMaterial, const char* pName,
                              const V& rValue) {
    s32 index = pMaterial->FindShaderParamIndex(pName);

    if (index != -1) {
        T* param = pMaterial->EditShaderParam<T>(index);
        *param = rValue;
    }
}
}  // namespace

namespace al {
/**
 * Constructs a sky projected with its own cameras.
 * @param pName actor name
 */
SkyProjection::SkyProjection(const char* pName) : Sky(pName) {
    mCamera0 = new sead::LookAtCamera(sead::Vector3f::zero, sead::Vector3f::ey, sead::Vector3f::ez);
    mCamera1 = new sead::LookAtCamera(sead::Vector3f::zero, sead::Vector3f::ey, sead::Vector3f::ez);
    mProjection0 = new sead::PerspectiveProjection(1.0f, 100000.0f, sead::Mathf::piHalf() / 2, 1.0f);
    mProjection1 = new sead::PerspectiveProjection(1.0f, 100000.0f, sead::Mathf::piHalf() / 2, 1.0f);
}

/**
 * Initializes the sky and its texture matrix parameters.
 * @param rInfo actor init info
 */
void SkyProjection::init(const ActorInitInfo& rInfo) {
    using SkyProjectionFunctor = FunctorV0M<SkyProjection*, void (SkyProjection::*)()>;

    Sky::init(rInfo);

    if (isSingleMode(rInfo)) {
        listenStageSwitchOnAppear(this, SkyProjectionFunctor(this, &SkyProjection::appear));
        listenStageSwitchOn(this, "SwitchDeadOn", SkyProjectionFunctor(this, &SkyProjection::kill));
    }

    tryGetArg(&mIsEnableTexMtxSet, rInfo, "IsEnableTexMtxSet");
    tryGetArg(&mIsEnableTexMtxSetUnder, rInfo, "IsEnableTexMtxSetUnder");
    tryGetArg(&mSkyTexMtxV, rInfo, "SkyTexMtxV");
    tryGetArg(&mSkyTexMtxVUnder, rInfo, "SkyTexMtxVUnder");
    tryGetArg(&mCloudTexMtxV, rInfo, "CloudTexMtxV");
    tryGetArg(&mCloudTexMtxVUnder, rInfo, "CloudTexMtxVUnder");
}

/**
 * Appears.
 */
void SkyProjection::appear() {
    LiveActor::appear();
}

/**
 * Kills the sky.
 */
void SkyProjection::kill() {
    LiveActor::kill();
}

/**
 * Follows the camera and updates the projection uniforms.
 */
void SkyProjection::control() {
    Sky::control();
    updateUniform();
}

/**
 * Updates the sky cameras and writes their matrices to the sky materials.
 */
void SkyProjection::updateUniform() {
    SimpleModelG3D* model = mModelKeeper->getModelCafe()->getModelG3D();

    for (s32 i = 0; i < model->getModelObj()->GetNumShapes(); i++) {
        nn::g3d::ModelObj* modelObj = model->getModelObj();
        const nn::g3d::ShapeObj* shape = modelObj->GetShape(i);
        nn::g3d::MaterialObj* material =
            modelObj->GetMaterial(shape->GetResource()->GetMaterialIndex());
        const nn::g3d::ResMaterial* resMaterial = material->GetResource();
        const char* shadingModelName = resMaterial->GetShaderAssign()->GetShadingModelName();

        if (!isEqualString("RenderSkyProjection", shadingModelName) &&
            !isEqualString("RenderSkyProjectionCubeMap", shadingModelName) &&
            !isEqualString("RenderSkyHDR", shadingModelName)) {
            continue;
        }

        f32 camera0Y = *resMaterial->FindRenderInfo("camera0_y")->GetFloat();
        f32 camera0Long = *resMaterial->FindRenderInfo("camera0_long")->GetFloat();
        f32 camera0Lati = *resMaterial->FindRenderInfo("camera0_lati")->GetFloat();
        f32 camera1Y = *resMaterial->FindRenderInfo("camera1_y")->GetFloat();
        f32 camera1Long = *resMaterial->FindRenderInfo("camera1_long")->GetFloat();
        f32 camera1Lati = *resMaterial->FindRenderInfo("camera1_lati")->GetFloat();
        f32 fovy0 = *resMaterial->FindRenderInfo("fovy0")->GetFloat();
        f32 fovy1 = *resMaterial->FindRenderInfo("fovy1")->GetFloat();
        f32 farDist0 = *resMaterial->FindRenderInfo("far_dist0")->GetFloat();
        f32 farDist1 = *resMaterial->FindRenderInfo("far_dist1")->GetFloat();

        sead::Vector3f dir0 = sead::Vector3f::ey;
        sead::Vector3f dir1 = sead::Vector3f::ey;
        rotateVectorDegreeX(&dir0, 90.0f - camera0Lati);
        rotateVectorDegreeY(&dir0, camera0Long);
        rotateVectorDegreeX(&dir1, 90.0f - camera1Lati);
        rotateVectorDegreeY(&dir1, camera1Long);

        mCamera0->setPos(sead::Vector3f(getTrans(this).x, camera0Y + getTrans(this).y,
                                        getTrans(this).z));
        mCamera0->setAt(mCamera0->getPos() + dir0);
        mCamera1->setPos(sead::Vector3f(getTrans(this).x, camera1Y + getTrans(this).y,
                                        getTrans(this).z));
        mCamera1->setAt(mCamera1->getPos() + dir1);
        mProjection0->setFovy(sead::Mathf::deg2rad(fovy0));
        mProjection1->setFovy(sead::Mathf::deg2rad(fovy1));
        mCamera0->updateViewMatrix();
        mCamera1->updateViewMatrix();

        const sead::Matrix44f& projMtx0 = mProjection0->getProjectionMatrix();
        const sead::Matrix44f& projMtx1 = mProjection1->getProjectionMatrix();
        sead::Matrix44f viewProjMtx0;
        viewProjMtx0.setMul(projMtx0, mCamera0->getMatrix());
        sead::Matrix44f viewProjMtx1;
        viewProjMtx1.setMul(projMtx1, mCamera1->getMatrix());

        trySetShaderParam<sead::Matrix44f>(material, "cSkyViewProj0", viewProjMtx0);

        trySetShaderParam<sead::Matrix44f>(material, "cSkyViewProj1", viewProjMtx1);

        trySetShaderParam<f32>(material, "cFarDist0", farDist0);

        trySetShaderParam<f32>(material, "cFarDist1", farDist1);

        if (isEqualString(resMaterial->GetName(), "Sky00Mat")) {
            trySetShaderParam<s32>(material, "cIsSetTexMtx", mIsEnableTexMtxSet);

            if (mIsEnableTexMtxSet) {
                trySetShaderParam<f32>(material, "cSkyTexMatV", mSkyTexMtxV);

                trySetShaderParam<f32>(material, "cCloudTexMatV", mCloudTexMtxV);
            }
        }

        if (isEqualString(resMaterial->GetName(), "Sky01Mat")) {
            trySetShaderParam<s32>(material, "cIsSetTexMtx", mIsEnableTexMtxSetUnder);

            if (mIsEnableTexMtxSetUnder) {
                trySetShaderParam<f32>(material, "cSkyTexMatV", mSkyTexMtxVUnder);

                trySetShaderParam<f32>(material, "cCloudTexMatV", mCloudTexMtxVUnder);
            }
        }
    }
}
}  // namespace al
