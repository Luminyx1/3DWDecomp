#include "Library/Joint/JointControllerKeeper.hpp"

#include "Library/Joint/JointAimInfo.hpp"
#include "Library/Joint/JointDirectionInfo.hpp"
#include "Library/Joint/JointLocalAxisRotator.hpp"
#include "Library/Joint/JointLocalDirController.hpp"
#include "Library/Joint/JointLocalQuatRotator.hpp"
#include "Library/Joint/JointLocalTransController.hpp"
#include "Library/Joint/JointLookAtController.hpp"
#include "Library/Joint/JointMasher.hpp"
#include "Library/Joint/JointMtxController.hpp"
#include "Library/Joint/JointQuatController.hpp"
#include "Library/Joint/JointRumbler.hpp"
#include "Library/Joint/JointSpringController.hpp"
#include "Library/Joint/JointTranslateShaker.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/ModelShapeUtil.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Project/Joint/JointAimController.hpp"

namespace {
bool sIsPausedJointControllers = false;
}  // namespace

namespace al {

static JointControllerKeeper* getJointControllerKeeper(const LiveActor* pActor) {
    return pActor->getModelKeeper()->getModelCafe()->getJointControllerKeeper();
}

static void registerJointController(const LiveActor* pActor, JointControllerBase* pController,
                                    s32 jointIndex) {
    pController->appendJointId(jointIndex);
    getJointControllerKeeper(pActor)->pushBackController(pController);
}

/**
 * Creates the joint controller keeper of the actor's model.
 * @param pActor Actor that owns the model.
 * @param maxControllers Maximum number of controllers.
 */
void initJointControllerKeeper(const LiveActor* pActor, s32 maxControllers) {
    pActor->getModelKeeper()->getModelCafe()->initJointControllerKeeper(maxControllers);
}

/**
 * Checks whether the actor's model has a joint controller keeper.
 * @param pActor Actor that owns the model.
 * @return Whether a keeper exists.
 */
bool isExistJointControllerKeeper(const LiveActor* pActor) {
    return getJointControllerKeeper(pActor) != nullptr;
}

/**
 * Registers local X, Y and Z axis rotators for one joint.
 * @param pActor Actor that owns the joint.
 * @param pRotate Rotation angles in degrees.
 * @param pJointName Name of the joint.
 */
void initJointLocalRotator(const LiveActor* pActor, sead::Vector3f* pRotate,
                           const char* pJointName) {
    initJointLocalXRotator(pActor, &pRotate->x, pJointName);
    initJointLocalYRotator(pActor, &pRotate->y, pJointName);
    initJointLocalZRotator(pActor, &pRotate->z, pJointName);
}

/**
 * Registers a local X axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointLocalXRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, sead::Vector3f::ex, true),
                            jointIndex);
}

/**
 * Registers a local Y axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointLocalYRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, sead::Vector3f::ey, true),
                            jointIndex);
}

/**
 * Registers a local Z axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointLocalZRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, sead::Vector3f::ez, true),
                            jointIndex);
}

/**
 * Registers a local axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param rAxis Rotation axis.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointLocalAxisRotator(const LiveActor* pActor, const sead::Vector3f& rAxis, f32* pDegree,
                               const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, rAxis, true), jointIndex);
}

/**
 * Registers an axis rotator for a joint and returns it.
 * @param pActor Actor that owns the joint.
 * @param rAxis Rotation axis.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 * @param isLocal Whether to rotate in joint-local space.
 * @return Created controller.
 */
JointLocalAxisRotator* initJointLocalAxisRotator_RS(const LiveActor* pActor,
                                                    const sead::Vector3f& rAxis, f32* pDegree,
                                                    const char* pJointName, bool isLocal) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    auto* rotator = new JointLocalAxisRotator(pDegree, rAxis, isLocal);
    registerJointController(pActor, rotator, jointIndex);
    return rotator;
}

/**
 * Registers a global X axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointGlobalXRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, sead::Vector3f::ex, false),
                            jointIndex);
}

/**
 * Registers a global axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param rAxis Rotation axis.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointGlobalAxisRotator(const LiveActor* pActor, const sead::Vector3f& rAxis,
                                f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, rAxis, false), jointIndex);
}

/**
 * Registers a global Y axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointGlobalYRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, sead::Vector3f::ey, false),
                            jointIndex);
}

/**
 * Registers a global Z axis rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pDegree Rotation angle in degrees.
 * @param pJointName Name of the joint.
 */
void initJointGlobalZRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalAxisRotator(pDegree, sead::Vector3f::ez, false),
                            jointIndex);
}

/**
 * Registers a local translation controller for a joint.
 * @param pActor Actor that owns the joint.
 * @param pTrans Local translation to apply.
 * @param pJointName Name of the joint.
 */
void initJointLocalTransController(const LiveActor* pActor, const sead::Vector3f* pTrans,
                                   const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalTransController(pActor, pTrans), jointIndex);
}

/**
 * Registers a controller that multiplies a joint by a matrix.
 * @param pActor Actor that owns the joint.
 * @param pMtx Matrix to apply.
 * @param pJointName Name of the joint.
 */
void initJointLocalMtxController(const LiveActor* pActor, const sead::Matrix34f* pMtx,
                                 const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointMtxController(pActor, pMtx, true), jointIndex);
}

/**
 * Registers a controller that replaces a joint matrix.
 * @param pActor Actor that owns the joint.
 * @param pMtx Matrix to apply.
 * @param pJointName Name of the joint.
 */
void initJointGlobalMtxController(const LiveActor* pActor, const sead::Matrix34f* pMtx,
                                  const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointMtxController(pActor, pMtx, false), jointIndex);
}

/**
 * Registers a controller that sets a joint rotation from a quaternion.
 * @param pActor Actor that owns the joint.
 * @param pQuat Rotation to apply.
 * @param pJointName Name of the joint.
 */
void initJointGlobalQuatController(const LiveActor* pActor, const sead::Quatf* pQuat,
                                   const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointQuatController(pActor, pQuat), jointIndex);
}

/**
 * Registers a direction controller for a joint.
 * @param pActor Actor that owns the joint.
 * @param pInfo Controller parameters.
 * @param pJointName Name of the joint.
 */
void initJointLocalDirController(const LiveActor* pActor, const JointDirectionInfo* pInfo,
                                 const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointLocalDirController(pInfo), jointIndex);
}

/**
 * Registers an aim controller for a joint.
 * @param pActor Actor that owns the joint.
 * @param pInfo Controller parameters.
 * @param pJointName Name of the joint.
 */
void initJointAimController(const LiveActor* pActor, const JointAimInfo* pInfo,
                            const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    registerJointController(pActor, new JointAimController(pInfo), jointIndex);
}

/**
 * Registers a translation shaker.
 * @param pActor Actor that owns the joint.
 * @param maxJoints Maximum number of joints.
 * @return Created controller.
 */
JointTranslateShaker* initJointTranslateShaker(const LiveActor* pActor, s32 maxJoints) {
    auto* shaker = new JointTranslateShaker(pActor, maxJoints);
    getJointControllerKeeper(pActor)->pushBackController(shaker);
    return shaker;
}

/**
 * Adds a joint shaken along X.
 * @param pShaker Shaker to append to.
 * @param pJointName Name of the joint.
 */
void appendJointTranslateShakerX(JointTranslateShaker* pShaker, const char* pJointName) {
    pShaker->append(pJointName, JointTranslateAxis_X);
}

/**
 * Adds a joint shaken along Y.
 * @param pShaker Shaker to append to.
 * @param pJointName Name of the joint.
 */
void appendJointTranslateShakerY(JointTranslateShaker* pShaker, const char* pJointName) {
    pShaker->append(pJointName, JointTranslateAxis_Y);
}

/**
 * Adds a joint shaken along Z.
 * @param pShaker Shaker to append to.
 * @param pJointName Name of the joint.
 */
void appendJointTranslateShakerZ(JointTranslateShaker* pShaker, const char* pJointName) {
    pShaker->append(pJointName, JointTranslateAxis_Z);
}

/**
 * Registers a joint masher.
 * @param pActor Actor that owns the joint.
 * @param pIsValid Flag enabling the controller.
 * @param maxJoints Maximum number of joints.
 * @return Created controller.
 */
JointMasher* initJointMasher(const LiveActor* pActor, const bool* pIsValid, s32 maxJoints) {
    auto* masher = new JointMasher(pActor, pIsValid, maxJoints);
    getJointControllerKeeper(pActor)->pushBackController(masher);
    return masher;
}

/**
 * Adds a joint to a masher.
 * @param pMasher Masher to append to.
 * @param pJointName Name of the joint.
 * @param rate Rate for the joint.
 */
void appendMashJoint(JointMasher* pMasher, const char* pJointName, f32 rate) {
    pMasher->append(pJointName, rate);
}

/**
 * Registers a joint rumbler.
 * @param pActor Actor that owns the joint.
 * @param pJointName Name of the joint.
 * @param cycle Oscillation cycles.
 * @param power Oscillation strength.
 * @param duration Length in steps.
 * @param startStep Step at which the effect starts.
 * @return Created controller.
 */
JointRumbler* initJointRumbler(const LiveActor* pActor, const char* pJointName, f32 cycle,
                               f32 power, u32 duration, s32 startStep) {
    auto* rumbler = new JointRumbler(pActor, pJointName, cycle, power, duration, startStep);
    getJointControllerKeeper(pActor)->pushBackController(rumbler);
    return rumbler;
}

/**
 * Registers a local quaternion rotator for a joint.
 * @param pActor Actor that owns the joint.
 * @param pJointName Name of the joint.
 * @param pQuat Rotation to apply.
 * @return Created controller.
 */
JointLocalQuatRotator* initJointLocalQuatRotator(const LiveActor* pActor, const char* pJointName,
                                                 const sead::Quatf* pQuat) {
    auto* rotator = new JointLocalQuatRotator(pActor, pJointName, pQuat);
    getJointControllerKeeper(pActor)->pushBackController(rotator);
    return rotator;
}

/**
 * Registers a look-at controller using the actor's base matrix.
 * @param pActor Actor that owns the joint.
 * @param maxJoints Maximum number of joints.
 * @return Created controller.
 */
JointLookAtController* initJointLookAtController(const LiveActor* pActor, s32 maxJoints) {
    auto* controller = new JointLookAtController(maxJoints, pActor->getBaseMtx());
    getJointControllerKeeper(pActor)->pushBackController(controller);
    return controller;
}

/**
 * Adds a joint to a look-at controller.
 * @param pController Controller to append to.
 * @param pActor Actor that owns the joint.
 * @param pJointName Name of the joint.
 * @param rate Rate for the joint.
 * @param rYawRange Yaw limits.
 * @param rPitchRange Pitch limits.
 * @param rLocalFront Local front direction.
 * @param rLocalUp Local up direction.
 */
void appendJointLookAtController(JointLookAtController* pController, const LiveActor* pActor,
                                 const char* pJointName, f32 rate, const sead::Vector2f& rYawRange,
                                 const sead::Vector2f& rPitchRange, const sead::Vector3f& rLocalFront,
                                 const sead::Vector3f& rLocalUp) {
    pController->appendJoint(getJointIndex(pActor->getModelKeeper(), pJointName), rate, rYawRange,
                             rPitchRange, rLocalFront, rLocalUp);
}

/**
 * Adds a joint to a look-at controller without a range judge.
 * @param pController Controller to append to.
 * @param pActor Actor that owns the joint.
 * @param pJointName Name of the joint.
 * @param rate Rate for the joint.
 * @param rYawRange Yaw limits.
 * @param rPitchRange Pitch limits.
 * @param rLocalFront Local front direction.
 * @param rLocalUp Local up direction.
 */
void appendJointLookAtControllerNoJudge(JointLookAtController* pController,
                                        const LiveActor* pActor, const char* pJointName, f32 rate,
                                        const sead::Vector2f& rYawRange,
                                        const sead::Vector2f& rPitchRange,
                                        const sead::Vector3f& rLocalFront,
                                        const sead::Vector3f& rLocalUp) {
    pController->appendJointNoJudge(getJointIndex(pActor->getModelKeeper(), pJointName), rate,
                                    rYawRange, rPitchRange, rLocalFront, rLocalUp);
}

/**
 * Registers a spring controller for a joint.
 * @param pActor Actor that owns the joint.
 * @param pJointName Name of the joint.
 * @return Created controller.
 */
JointSpringController* initJointSpringController(const LiveActor* pActor, const char* pJointName) {
    s32 jointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    auto* controller = new JointSpringController();
    registerJointController(pActor, controller, jointIndex);
    return controller;
}

/**
 * Pauses all joint controllers.
 */
void pauseJointControllers() {
    sIsPausedJointControllers = true;
}

/**
 * Resumes all joint controllers.
 */
void resumeJointControllers() {
    sIsPausedJointControllers = false;
}

/**
 * Checks whether joint controllers are paused.
 * @return Whether controllers are paused.
 */
bool isPausedJointControllers() {
    return sIsPausedJointControllers;
}

}  // namespace al
