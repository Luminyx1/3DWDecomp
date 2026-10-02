#include "Library/Play/Camera/CameraPoserAnim.hpp"

#include <gfx/seadCamera.h>
#include <nn/g3d/g3d_ResSceneAnim.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Resource/Resource.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Library/Camera/CameraSwitcher.hpp"

/**
 * Looks up a camera animation of the scene animation by name.
 * @param pName Name of the camera animation.
 * @return The camera animation, or nullptr if it does not exist.
 */
__attribute__((noinline)) inline const nn::g3d::ResCameraAnim*
nn::g3d::ResSceneAnim::FindCameraAnim(const char* pName) const {
    const nn::util::ResDic* dictionary =
        reinterpret_cast<const nn::util::ResDic*>(mCameraAnimDictOffset);

    if (dictionary == nullptr) {
        return nullptr;
    }

    int index = dictionary->FindIndex(pName);

    if (index == nn::util::ResDic::Npos) {
        return nullptr;
    }

    return &reinterpret_cast<const ResCameraAnim*>(mCameraAnimOffset)[index];
}

namespace al {

/**
 * Creates a camera that plays back a camera animation.
 * @param pSwitcher Switcher that controls this camera.
 */
CameraPoserAnim::CameraPoserAnim(CameraSwitcher* pSwitcher) : mSwitcher(pSwitcher) {
    mName = "Anim";
}

/**
 * Loads the animation file of the resource.
 * @param pResource Resource that holds the animation file.
 * @param rId Placement id of the camera.
 * @param rInfo Actor init info.
 */
void CameraPoserAnim::initAnim(const Resource* pResource, const PlacementId& rId,
                               const ActorInitInfo& rInfo) {
    mPlacementId = new PlacementId(rId);
    StringTmp<128> fileName("%s.bfres", pResource->getArchiveName());
    mResFile = nn::g3d::ResFile::ResCast(pResource->getOtherFile(fileName, nullptr));
    mIsInitAnim = true;
}

/**
 * Sets the animation to play and the matrix it is played relative to.
 * @param pName Name of the scene animation.
 * @param pBaseMtx Matrix the animation is played relative to.
 */
void CameraPoserAnim::setAnimAndBaseMtx(const char* pName, const sead::Matrix34f* pBaseMtx) {
    mBaseMtx = pBaseMtx;
    const nn::g3d::ResSceneAnim* sceneAnim = mResFile->FindSceneAnim(pName);

    if (sceneAnim == nullptr) {
        return;
    }

    mCameraAnim = sceneAnim->FindCameraAnim("AnimCamera");

    if (mCameraAnim == nullptr) {
        return;
    }

    mMaxFrame = mCameraAnim->frameCount;
    mCameraAnim->Initialize(&mAnimResult);
    mIsSetAnim = true;
}

/**
 * Gets the number of frames of a scene animation.
 * @param pName Name of the scene animation.
 * @return Frame count of the animation, or 0 if it has no camera animation.
 */
s32 CameraPoserAnim::getMaxFrame(const char* pName) const {
    const nn::g3d::ResCameraAnim* cameraAnim =
        mResFile->FindSceneAnim(pName)->FindCameraAnim("AnimCamera");

    if (cameraAnim == nullptr) {
        return 0;
    }

    return cameraAnim->frameCount;
}

/**
 * Restarts the animation, or ends the camera if no animation is set.
 */
void CameraPoserAnim::start() {
    if (mIsInitAnim && mIsSetAnim) {
        mFrame = 0;
        return;
    }

    mSwitcher->end(*mPlacementId, -1);
}

/**
 * Advances the animation and evaluates the camera pose of the new frame.
 */
void CameraPoserAnim::update() {
    mFrame++;

    if (mMaxFrame <= mFrame) {
        return;
    }

    mCameraAnim->Evaluate(&mAnimResult, static_cast<f32>(mFrame));

    const sead::Matrix34f& baseMtx = *mBaseMtx;
    const sead::Vector3f& animPos =
        *reinterpret_cast<const sead::Vector3f*>(&mAnimResult.values[4]);
    const sead::Vector3f& animAim =
        *reinterpret_cast<const sead::Vector3f*>(&mAnimResult.values[7]);
    sead::Vector3f trans;
    baseMtx.getTranslation(trans);
    mCameraPos.set(animPos.x, animPos.y, animPos.z);
    mCameraPos.setRotated(baseMtx, mCameraPos);
    mCameraPos += trans;
    mLookAtPos.set(animAim.x, animAim.y, animAim.z);
    mLookAtPos.setRotated(baseMtx, mLookAtPos);
    mLookAtPos += trans;

    if (!mIsApplyAnimFovyAndTwist) {
        return;
    }

    setFovyDegree(sead::Mathf::rad2deg(mAnimResult.values[3]));

    sead::Vector3f front = mLookAtPos - mCameraPos;
    normalize(&front);
    sead::Vector3f up = sead::Vector3f::ey;

    if (isParallelDirection(front, up, 0.01f)) {
        return;
    }

    sead::Vector3f side;
    side.setCross(front, up);
    sead::Vector3f cameraUp;
    cameraUp.setCross(side, front);
    sead::Vector3f back = -front;
    rotateVectorDegree(&cameraUp, cameraUp, back, sead::Mathf::rad2deg(mAnimResult.values[10]));
    setUp(cameraUp);
}

/**
 * Applies the camera pose to a look-at camera.
 * @param pCamera Camera to write the pose to.
 */
void CameraPoserAnim::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    if (isNearZero(mCameraPos - mLookAtPos, 0.01f)) {
        return;
    }

    pCamera->setPos(mCameraPos);
    pCamera->setAt(mLookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Checks whether this camera is the active camera of the switcher.
 * @return Whether this camera is active.
 */
bool CameraPoserAnim::isCameraCurrent() const {
    return mSwitcher->isCameraCurrent(this);
}

/**
 * Checks whether the animation has finished.
 * @return Whether the animation has finished.
 */
bool CameraPoserAnim::isEndAnim() const {
    return mMaxFrame <= mFrame;
}

/**
 * Checks whether the resource contains a scene animation.
 * @param pName Name of the scene animation.
 * @return Whether the animation exists.
 */
bool CameraPoserAnim::isExistAnim(const char* pName) const {
    return mResFile->FindSceneAnim(pName) != nullptr;
}

}  // namespace al
