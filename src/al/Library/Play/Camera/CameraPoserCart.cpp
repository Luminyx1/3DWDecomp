#include "Library/Play/Camera/CameraPoserCart.hpp"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

// The nerve keeper of a camera poser holds the poser itself, not its IUseNerve base.
#define POSER_NERVE_DECL(Class, Action)                                                            \
    class Class##Nrv##Action : public al::Nerve {                                                  \
    public:                                                                                        \
        void execute(al::NerveKeeper* pKeeper) const override {                                    \
            static_cast<Class*>(static_cast<void*>(pKeeper->getParent<al::IUseNerve>()))           \
                ->exe##Action();                                                                   \
        }                                                                                          \
    };

POSER_NERVE_DECL(CameraPoserCart, Follow)
POSER_NERVE_DECL(CameraPoserCart, Stop)

NERVES_MAKE_NOSTRUCT(CameraPoserCart, Follow, Stop)

}  // namespace

namespace al {

/**
 * Constructs a camera that follows a cart from behind.
 * @param pName Poser name.
 */
CameraPoserCart::CameraPoserCart(const char* pName) : CameraPoser_RS(pName) {
    initNerve(&NrvCameraPoserCartFollow, 0);
}

/**
 * Starts following the cart.
 * @param rInfo Camera start info.
 */
void CameraPoserCart::start(const CameraStartInfo& rInfo) {
    setNerve(this, &NrvCameraPoserCartFollow);
}

/**
 * Stops moving the camera.
 */
void CameraPoserCart::stop() {
    setNerve(this, &NrvCameraPoserCartStop);
}

/**
 * Resumes following the cart.
 */
void CameraPoserCart::restart() {
    setNerve(this, &NrvCameraPoserCartFollow);
}

/**
 * Keeps the camera behind the cart, turning it towards the moving direction or towards the
 * requested destination angle.
 */
void CameraPoserCart::exeFollow() {
    mUp.set(sead::Vector3f::ey);
    alCameraPoserFunction::setLookAtPosToTargetAddOffset(this, {0.0f, mLookAtOffsetY, 0.0f});

    sead::Vector3f velocity = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetVelocity(&velocity, this);

    sead::Vector3f dir;

    if (isFirstStep(this)) {
        const sead::Vector3f& rPreCameraPos = alCameraPoserFunction::getPreCameraPos(this);
        mDirH = {rPreCameraPos.x - mAt.x, 0.0f, rPreCameraPos.z - mAt.z};
        normalize(&mDirH);

        if (isNearZero(velocity, 0.05f)) {
            setNerve(this, &NrvCameraPoserCartFollow);
            return;
        }

        sead::Vector3f front = {-mDirH.x, 0.0f, -mDirH.z};

        if (!tryNormalizeOrZero(&front)) {
            alCameraPoserFunction::calcTargetFront(&front, this);

            if (isNearZero(front, 0.05f)) {
                setNerve(this, &NrvCameraPoserCartFollow);
                return;
            }
        }

        dir = -front;
        sead::Vector3f side;
        side.setCross(dir, mUp);

        if (isNearZero(side, 0.05f)) {
            setNerve(this, &NrvCameraPoserCartFollow);
            return;
        }

        rotateVectorDegree(&dir, dir, side, mAngleDegreeV);
    } else {
        if (mDestinationStep >= 1) {
            sead::Vector3f destination = sead::Vector3f::ez;
            rotateVectorDegreeY(&destination, mDestinationAngleDegree);
            f32 angle = calcAngleDegree(mDirH, destination);
            f32 rate = sead::Mathf::clamp(
                static_cast<f32>(getNerveStep(this)) / static_cast<f32>(mDestinationStep), 0.0f,
                1.0f);
            turnVecToVecDegree(&dir, mDirH, destination, angle * rate);
        } else {
            dir.set(mEye.x - mAt.x, 0.0f, mEye.z - mAt.z);
            normalize(&dir);
            sead::Vector3f velocityH = velocity;
            velocityH.y = 0.0f;

            if (tryNormalizeOrZero(&velocityH)) {
                sead::Quatf quat = sead::Quatf::unit;
                makeQuatFrontUp(&quat, dir, mUp);
                turnQuatZDirRate(&quat, quat, -velocityH, 0.06f);
                calcQuatFront(&dir, quat);
            }
        }

        sead::Vector3f side;
        side.setCross(dir, mUp);
        rotateVectorDegree(&dir, dir, side, mAngleDegreeV);
    }

    mEye = mAt + dir * mDistance;
}

/**
 * Keeps the camera where it is.
 */
void CameraPoserCart::exeStop() {}

/**
 * Turns the camera towards a fixed horizontal angle over a number of steps.
 * @param angleDegree Destination angle around the Y axis, in degrees.
 * @param step Number of steps to reach the destination angle.
 */
void CameraPoserCart::setUseDestinationAngle(f32 angleDegree, s32 step) {
    mDestinationAngleDegree = angleDegree;
    mDestinationStep = step;
}

}  // namespace al
