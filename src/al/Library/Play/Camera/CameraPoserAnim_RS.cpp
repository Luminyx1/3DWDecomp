#include "Library/Play/Camera/CameraPoserAnim_RS.hpp"

#include <nn/g3d/g3d_ResSceneAnim.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Project/Base/StringUtil.hpp"

/**
 * Looks up a camera animation of the scene animation by name.
 * @param pName Name of the camera animation.
 * @return The camera animation, or nullptr if it does not exist.
 */
inline const nn::g3d::ResCameraAnim*
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
 * Creates a camera that plays back a camera animation from a resource.
 */
CameraPoserAnim_RS::CameraPoserAnim_RS() : CameraPoser_RS("アニメ") {
    alCameraPoserFunction::initAngleSwing(this);
    alCameraPoserFunction::invalidateCameraBlur(this);
}

/**
 * Loads the animation file of the resource and sets the base matrix of the animation.
 * @param pResource Resource that holds the animation file.
 * @param pBaseMtx Matrix the animation is played relative to.
 */
void CameraPoserAnim_RS::initAnimResource(const Resource* pResource,
                                          const sead::Matrix34f* pBaseMtx) {
    StringTmp<128> fileName("%s.bfres", pResource->getArchiveName());
    mResFile = nn::g3d::ResFile::ResCast(pResource->getOtherFile(fileName, nullptr));
    mBaseMtxPtr = pBaseMtx;
}

/**
 * Starts playing an animation.
 * @param pName Name of the scene animation.
 * @param startStep First frame to play, or a negative value to start at the beginning.
 * @param endStep Last frame to play, or a negative value to play until the end.
 * @param playStep Number of steps the playback takes, or a negative value to play at normal speed.
 */
void CameraPoserAnim_RS::setAnim(const char* pName, s32 startStep, s32 endStep, s32 playStep) {
    mCameraAnim = mResFile->FindSceneAnim(pName)->FindCameraAnim("AnimCamera");
    mStep = 0;
    mStepMax = mCameraAnim->frameCount;
    mStartStep = sead::Mathi::max(startStep, 0);
    mEndStep = endStep >= 0 ? endStep : mStepMax;
    mAnimName = pName;
    mPlayStep = playStep >= 0 ? playStep : mEndStep - mStartStep;
    mCameraAnim->Initialize(&mAnimResult);
}

/**
 * Checks whether the resource contains a scene animation.
 * @param pName Name of the scene animation.
 * @return Whether the animation exists.
 */
bool CameraPoserAnim_RS::isExistAnim(const char* pName) const {
    return mResFile->FindSceneAnim(pName) != nullptr;
}

/**
 * Prepares the camera for playback and disables the player's camera input.
 * @param rInfo Start info.
 */
void CameraPoserAnim_RS::start(const CameraStartInfo& rInfo) {
    _1b0 = 0;
    mIsValidZoom = !alCameraPoserFunction::isHoldCameraZoom(this);
    alCameraPoserFunction::disableInput(this, true);
}

/**
 * Evaluates the current frame of the animation and advances it.
 */
void CameraPoserAnim_RS::update() {
    if (alCameraPoserFunction::isSnapShotMode(this)) {
        return;
    }

    if (mStep < 0) {
        return;
    }

    f32 rate = normalize(static_cast<f32>(mStep), 0.0f, static_cast<f32>(mPlayStep));
    f32 frame = lerpValue(rate, static_cast<f32>(mStartStep), static_cast<f32>(mEndStep));
    mCameraAnim->Evaluate(&mAnimResult, frame);

    if (mIsCheckRange) {
        mFovyDegree = sead::Mathf::rad2deg(mAnimResult.values[3]);
    }

    const sead::Matrix34f& baseMtx = *mBaseMtxPtr;
    const sead::Vector3f& animPos =
        *reinterpret_cast<const sead::Vector3f*>(&mAnimResult.values[4]);
    const sead::Vector3f& animAim =
        *reinterpret_cast<const sead::Vector3f*>(&mAnimResult.values[7]);
    sead::Vector3f trans;
    baseMtx.getTranslation(trans);
    sead::Vector3f eye;
    eye.setRotated(baseMtx, animPos);
    mEye = trans + eye;
    sead::Vector3f at;
    at.setRotated(baseMtx, animAim);
    mAt = trans + at + sead::Vector3f(0.0f, 0.0f, 0.0f) + mLookAtOffset;

    sead::Vector3f up = sead::Vector3f::ey;

    if (mIsRotateBaseUp) {
        up.setRotated(baseMtx, up);
    }

    sead::Vector3f dir = mAt - mEye;
    sead::Vector3f front = dir;
    normalize(&front);

    if (!isParallelDirection(front, up, 0.01f)) {
        sead::Vector3f side;
        side.setCross(front, up);
        sead::Vector3f cameraUp;
        cameraUp.setCross(side, front);
        sead::Vector3f back = -front;
        rotateVectorDegree(&cameraUp, cameraUp, back,
                           sead::Mathf::rad2deg(mAnimResult.values[10]));
        mUp.e = cameraUp.e;
    }

    mAt = mEye + dir;
    mStep++;

    if (mPlayStep <= mStep) {
        if (mCameraAnim->flags & 4) {
            mStep = 0;
        } else {
            setAnimEnd();
        }
    }
}

/**
 * Stops the animation and gives the camera input back to the player.
 */
void CameraPoserAnim_RS::setAnimEnd() {
    mStep = -1;
    alCameraPoserFunction::disableInput(this, false);
}

/**
 * Calculates the number of frames of a scene animation.
 * @param pName Name of the scene animation.
 * @return Frame count of the animation, or 0 if it has no camera animation.
 */
s32 CameraPoserAnim_RS::calcStepMax(const char* pName) const {
    const nn::g3d::ResCameraAnim* cameraAnim =
        mResFile->FindSceneAnim(pName)->FindCameraAnim("AnimCamera");

    if (cameraAnim == nullptr) {
        return 0;
    }

    return cameraAnim->frameCount;
}

/**
 * Checks whether an animation is being played.
 * @param pName Name of the scene animation.
 * @return Whether the animation is playing.
 */
bool CameraPoserAnim_RS::isAnimPlaying(const char* pName) const {
    if (mStep < 0) {
        return false;
    }

    return isEqualString(mAnimName, pName);
}

/**
 * Checks whether the animation has finished.
 * @return Whether the animation has finished.
 */
bool CameraPoserAnim_RS::isAnimEnd() const {
    if (mStep < 0) {
        return true;
    }

    return mCameraAnim == nullptr;
}

}  // namespace al
