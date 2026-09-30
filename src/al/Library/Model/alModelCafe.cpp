#include "Library/Model/alModelCafe.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ShapeObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/util/util_MatrixApi.h>

#include "Library/Anim/AnimPlayerSkl.hpp"
#include "Library/Joint/JointControllerKeeper.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunc.hpp"
#include "Library/SaveData/ActorInitResourceData.hpp"
#include "Project/Anim/AnimPlayerSimple.hpp"
#include "Project/Anim/InitResourceDataAnim.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace {

nn::g3d::ResFile* getResFile(const al::Resource* pResource) {
    return reinterpret_cast<nn::g3d::ResFile*>(pResource->_C0);
}

al::Resource* getTextureResource(const al::Resource* pResource) {
    return pResource->mPatchRes;
}

al::ActorInitResourceData* getInitResourceData(const al::Resource* pResource) {
    return reinterpret_cast<al::ActorInitResourceData*>(pResource->_B0);
}

u16 getResFileCount(const nn::g3d::ResFile* pResFile, s32 offset) {
    return *reinterpret_cast<const u16*>(reinterpret_cast<const u8*>(pResFile) + offset);
}

const nn::g3d::ResModel* getFirstResModel(const nn::g3d::ResFile* pResFile) {
    return *reinterpret_cast<nn::g3d::ResModel* const*>(reinterpret_cast<const u8*>(pResFile) + 0x28);
}

const nn::g3d::ShapeObj* getShapeObj(const nn::g3d::ModelObj* pModelObj, s32 index) {
    return &(*reinterpret_cast<nn::g3d::ShapeObj* const*>(reinterpret_cast<const u8*>(pModelObj) +
                                                          0x40))[index];
}

void storeWorldMtx(sead::Matrix34f* pOut, const nn::util::Matrix4x3fType& rMtx) {
    nn::util::MatrixStore(reinterpret_cast<nn::util::FloatColumnMajor4x3*>(pOut), rMtx);
}

}  // namespace

inline alModelCafe::alModelCafe(const alModelCafe* pOther) {
    mIsCreatedFromOther = true;
    mModelName = pOther->mModelName;
    mFileName = pOther->mFileName;
    mModelG3D = al::SimpleModelG3D::createFromOtherModel(pOther->mModelG3D);
    mModelRes = pOther->mModelRes;
    mTextureRes = pOther->mTextureRes;
    mAnimRes = pOther->mAnimRes;
    mAnimPlayerSkl = pOther->mAnimPlayerSkl;
    mAnimPlayerMat1 = pOther->mAnimPlayerMat1;
    mAnimPlayerMat2 = pOther->mAnimPlayerMat2;
    mAnimPlayerMat0 = pOther->mAnimPlayerMat0;
    mAnimPlayerVis = pOther->mAnimPlayerVis;
    mJointControllerKeeper = pOther->mJointControllerKeeper;
    mBaseMtx = pOther->mBaseMtx;
    mViewMtx = pOther->mViewMtx;
    mInvViewMtx = pOther->mInvViewMtx;
    mProjMtx = pOther->mProjMtx;
    mViewProjMtx = pOther->mViewProjMtx;
}

/**
 * Creates a model sharing the resources and animation players of another model.
 * @param pOther Model to copy from.
 * @return New model.
 */
alModelCafe* alModelCafe::createFromOtherModel(alModelCafe* pOther) {
    return new alModelCafe(static_cast<const alModelCafe*>(pOther));
}

/**
 * Constructs an empty model.
 * @param isCreateBaseMtx Whether to allocate the base matrix.
 */
alModelCafe::alModelCafe(bool isCreateBaseMtx)
    : mIsCreatedFromOther(false), mModelName(nullptr), mFileName(nullptr), mModelG3D(nullptr),
      mModelRes(nullptr), mTextureRes(nullptr), mAnimRes(nullptr),
      mAnimPlayerSkl(nullptr), mAnimPlayerMat1(nullptr), mAnimPlayerMat2(nullptr),
      mAnimPlayerMat0(nullptr), mAnimPlayerVis(nullptr), mJointControllerKeeper(nullptr),
      mBaseMtx(nullptr), mViewMtx(nullptr), mInvViewMtx(nullptr), mProjMtx(nullptr) {
    if (isCreateBaseMtx) {
        mBaseMtx = new sead::Matrix34f(sead::Matrix34f::ident);
    }
}

/**
 * Constructs an empty model with a base matrix.
 */
alModelCafe::alModelCafe()
    : mIsCreatedFromOther(false), mModelName(nullptr), mFileName(nullptr), mModelG3D(nullptr),
      mModelRes(nullptr), mTextureRes(nullptr), mAnimRes(nullptr),
      mAnimPlayerSkl(nullptr), mAnimPlayerMat1(nullptr), mAnimPlayerMat2(nullptr),
      mAnimPlayerMat0(nullptr), mAnimPlayerVis(nullptr), mJointControllerKeeper(nullptr),
      mBaseMtx(nullptr), mViewMtx(nullptr), mInvViewMtx(nullptr), mProjMtx(nullptr) {
    mBaseMtx = new sead::Matrix34f(sead::Matrix34f::ident);
}

/**
 * Destroys the model.
 */
alModelCafe::~alModelCafe() {
    if (!mIsCreatedFromOther && mBaseMtx) {
        delete mBaseMtx;
        mBaseMtx = nullptr;
    }

    if (mModelG3D) {
        delete mModelG3D;
        mModelG3D = nullptr;
    }
}

/**
 * Loads the model, texture and animation resources.
 * @param pModelArcName Model archive name.
 * @param pAnimArcName Animation archive name.
 * @param pTexArcName Texture archive name.
 */
void alModelCafe::initResource(const char* pModelArcName, const char* pAnimArcName,
                               const char* pTexArcName) {
    mModelRes = al::findOrCreateResource(pModelArcName, nullptr);

    nn::g3d::ResFile* texResFile = nullptr;
    al::Resource* modelRes;
    if (pTexArcName) {
        al::Resource* texRes = al::findOrCreateResource(pTexArcName, nullptr);
        al::StringTmp<256> texFileName("%s.bfres", al::getBaseName(pTexArcName));
        texRes->tryCreateResGraphicsFile(texFileName, nullptr);
        texResFile = getResFile(texRes);
        modelRes = mModelRes;
    } else {
        modelRes = mModelRes;
    }

    {
        al::StringTmp<256> fileName("%s.bfres", al::getBaseName(pModelArcName));
        modelRes->tryCreateResGraphicsFile(fileName, texResFile);
    }

    al::Resource* texRes;
    if (mModelRes && (texRes = getTextureResource(mModelRes)) && getResFile(texRes) &&
        getResFileCount(getResFile(texRes), 0xdc) != 0) {
        mTextureRes = texRes;
        al::StringTmp<256> texFileName("%s.bfres", al::getBaseName(pModelArcName));
        if (!texResFile) {
            texResFile = getResFile(mModelRes);
        }

        texRes->tryCreateResGraphicsFile(texFileName, texResFile);
    }

    if (pAnimArcName) {
        mModelName = al::createStringIfInStack(al::getBaseName(pAnimArcName));
        mFileName = al::createStringIfInStack(pAnimArcName);
        al::Resource* animRes = al::findOrCreateResource(pAnimArcName, nullptr);
        mAnimRes = animRes;
        al::StringTmp<256> animFileName("%s.bfres", mModelName);
        animRes->tryCreateResGraphicsFile(animFileName, nullptr);
    }

    if (!mFileName) {
        mFileName = al::createStringIfInStack(pModelArcName);
    }
}

/**
 * Creates the model object and the animation players.
 * @param bufferNum Number of GPU buffers.
 * @param pAllocator GPU memory allocator.
 */
void alModelCafe::initModel(s32 bufferNum, al::GpuMemAllocator* pAllocator) {
    mModelG3D = new al::SimpleModelG3D();
    mModelG3D->initResource(mTextureRes ? mTextureRes : mModelRes, 1, nullptr, pAllocator);

    al::Resource* texRes = getTextureResource(mModelRes);
    nn::g3d::ResFile* texResFile;
    if (texRes && (texResFile = getResFile(texRes)) &&
        (getResFileCount(texResFile, 0xe2) != 0 || getResFileCount(texResFile, 0xe4) != 0 ||
         getResFileCount(texResFile, 0xe6) != 0)) {
        mInitResourceDataAnim = al::InitResourceDataAnim::tryCreate(mModelRes, mAnimRes, texRes);
    } else if (mAnimRes) {
        mInitResourceDataAnim = al::InitResourceDataAnim::tryCreate(mModelRes, mAnimRes, nullptr);
    } else {
        mInitResourceDataAnim = getInitResourceData(mModelRes)->getAnimData();
    }

    sead::Matrix34f mtx;
    mtx.makeT(sead::Vector3f::zero);
    mModelG3D->updateWorldMatrix(mtx, sead::Vector3f::ones);

    al::AnimPlayerInitInfo info = {mAnimRes ? mAnimRes : mModelRes, mModelG3D->getModelObj(),
                                   mModelRes, mInitResourceDataAnim};

    mAnimPlayerSkl = al::AnimPlayerSkl::tryCreate(&info, bufferNum);
    mAnimPlayerMat1 = al::AnimPlayerMat::tryCreate(&info, 1);
    al::AnimPlayerMat* mat0 = al::AnimPlayerMat::tryCreate(&info, 0);
    mAnimPlayerMat0 = mat0;
    al::AnimPlayerMat* mat2 = al::AnimPlayerMat::tryCreate(&info, 2);
    mAnimPlayerMat2 = mat2;
    mAnimPlayerVis = al::AnimPlayerVis::tryCreate(&info);

    if (mAnimPlayerSkl) {
        mAnimPlayerSkl->initInterp(mFileName);
    }

    if (!mat0 && !mat2) {
        mModelG3D->tryInitFixedMatUbo();
    }

    const nn::g3d::SkeletonObj* skeleton = mModelG3D->getModelObj()->GetSkeleton();
    if (skeleton && skeleton->GetBoneCount() != 0) {
        mWorldMtxArray = new sead::Matrix34f[skeleton->GetBoneCount()];
        u32 boneNum = skeleton->GetBoneCount();
        for (u32 i = 0; i < boneNum; i++) {
            storeWorldMtx(&mWorldMtxArray[i], skeleton->GetWorldMtxArray()[i]);
        }
    }
}

/**
 * Gets the resource containing the animations.
 * @return Animation resource, or the model resource if there is none.
 */
const al::Resource* alModelCafe::getAnimResource() const {
    return mAnimRes ? mAnimRes : mModelRes;
}

/**
 * Shows the model.
 */
void alModelCafe::show() {
    mModelG3D->mIsVisible = true;
    if (mAnimPlayerSkl) {
        mAnimPlayerSkl->reset();
    }
}

/**
 * Checks whether the model is hidden.
 * @return Whether the model is hidden.
 */
bool alModelCafe::isHidden() const {
    return !mModelG3D->mIsVisible;
}

/**
 * Hides the model.
 */
void alModelCafe::hide() {
    mModelG3D->mIsVisible = false;
}

/**
 * Updates the GPU buffers and the animation players.
 */
void alModelCafe::update() {
    mModelG3D->swapGPUBuffer();
    if (mAnimPlayerSkl) {
        mAnimPlayerSkl->update();
    }

    if (mAnimPlayerMat1) {
        mAnimPlayerMat1->update();
    }

    if (mAnimPlayerMat0) {
        mAnimPlayerMat0->update();
    }

    if (mAnimPlayerMat2) {
        mAnimPlayerMat2->update();
    }

    if (mAnimPlayerVis) {
        mAnimPlayerVis->update();
    }
}

/**
 * Updates the GPU buffers while paused.
 */
void alModelCafe::updatePaused() {
    mModelG3D->swapGPUBuffer();
}

/**
 * Runs the last update step of the animation players.
 */
void alModelCafe::updateLast() {
    if (mAnimPlayerSkl) {
        mAnimPlayerSkl->updateLast();
    }

    if (mAnimPlayerMat1) {
        mAnimPlayerMat1->updateLast();
    }

    if (mAnimPlayerMat0) {
        mAnimPlayerMat0->updateLast();
    }

    if (mAnimPlayerMat2) {
        mAnimPlayerMat2->updateLast();
    }

    if (mAnimPlayerVis) {
        mAnimPlayerVis->updateLast();
    }
}

/**
 * Calculates animations and world matrices.
 * @param rMtx Base matrix.
 * @param rScale Scale.
 */
void alModelCafe::calc(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale) {
    if (mAnimPlayerSkl) {
        mAnimPlayerSkl->calcSklAnim();
    }

    if (mAnimPlayerSkl) {
        mAnimPlayerSkl->calcNeedUpdateAnimNext();
    }

    if (mAnimPlayerMat1) {
        mAnimPlayerMat1->calcNeedUpdateAnimNext();
    }

    if (mAnimPlayerMat0) {
        mAnimPlayerMat0->calcNeedUpdateAnimNext();
    }

    if (mAnimPlayerMat2) {
        mAnimPlayerMat2->calcNeedUpdateAnimNext();
    }

    if (mAnimPlayerVis) {
        mAnimPlayerVis->calcNeedUpdateAnimNext();
    }

    mModelG3D->updateWorldMatrix(rMtx, rScale);
    mModelG3D->updateGPUBuffer(mViewMtx);
    *mBaseMtx = rMtx;

    if (!mWorldMtxArray) {
        return;
    }

    const nn::g3d::SkeletonObj* skeleton = mModelG3D->getModelObj()->GetSkeleton();
    if (!skeleton) {
        return;
    }

    u32 boneNum = skeleton->GetBoneCount();
    for (u32 i = 0; i < boneNum; i++) {
        storeWorldMtx(&mWorldMtxArray[i], skeleton->GetWorldMtxArray()[i]);
    }
}

/**
 * Sets the camera matrices used for drawing.
 * @param pViewMtx View matrix.
 * @param pInvViewMtx Inverse view matrix.
 * @param pProjMtx Projection matrix.
 * @param pViewProjMtx View projection matrix.
 */
void alModelCafe::setCameraInfo(const sead::Matrix34f* pViewMtx, const sead::Matrix34f* pInvViewMtx,
                                const sead::Matrix44f* pProjMtx,
                                const sead::Matrix44f* pViewProjMtx) {
    mViewMtx = pViewMtx;
    mInvViewMtx = pInvViewMtx;
    mProjMtx = pProjMtx;
    mViewProjMtx = pViewProjMtx;
}

/**
 * Gets a joint world matrix.
 * @param index Joint index.
 * @return Joint world matrix.
 */
const sead::Matrix34f* alModelCafe::getWorldMtxPtrByIndex(s32 index) const {
    return &mWorldMtxArray[index];
}

/**
 * Gets the model resource.
 * @return Model resource.
 */
const nn::g3d::ResModel* alModelCafe::getResModel() const {
    if (mTextureRes) {
        return getFirstResModel(getResFile(mTextureRes));
    }

    return getFirstResModel(getResFile(mModelRes));
}

/**
 * Enables bounding updates if any shape has a bounding node.
 */
void alModelCafe::initUpdateBounding() {
    const nn::g3d::ModelObj* modelObj = mModelG3D->getModelObj();
    s32 shapeNum = modelObj->GetNumShapes();
    bool isExist = false;
    for (s32 i = 0; i < shapeNum; i++) {
        bool isExistNode = alModelFunction::isExistBoundingNode(getShapeObj(modelObj, i)->GetResource());
        isExist |= isExistNode;
        if (isExistNode) {
            break;
        }
    }

    mModelG3D->_44 = isExist;
}

/**
 * Creates the joint controller keeper and registers it to the skeleton.
 * @param num Maximum number of joint controllers.
 */
void alModelCafe::initJointControllerKeeper(s32 num) {
    mJointControllerKeeper = new al::JointControllerKeeper(num);
    mModelG3D->getModelObj()->GetSkeleton()->SetCalculateWorldCallback(mJointControllerKeeper);
}
