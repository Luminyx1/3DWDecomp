#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <nn/g3d/g3d_SkeletonObj.h>

namespace al {
class JointAimInfo;
class JointControllerBase;
class JointDirectionInfo;
class JointLocalAxisRotator;
class JointLocalQuatRotator;
class JointLookAtController;
class JointRumbler;
class JointMasher;
class JointSpringController;
class JointTranslateShaker;
class LiveActor;

class JointControllerKeeper : public nn::g3d::ICalculateWorldCallback {
public:
    JointControllerKeeper(s32 maxControllers);

    void Exec(CallbackArg& rArg, nn::g3d::WorldMtxManip& rManip) override;
    void pushBackController(JointControllerBase* pController);

private:
    sead::ObjArray<JointControllerBase*> mControllers;
};

static_assert(sizeof(JointControllerKeeper) == 0x28);

void initJointControllerKeeper(const LiveActor* pActor, s32 maxControllers);
bool isExistJointControllerKeeper(const LiveActor* pActor);
void initJointLocalRotator(const LiveActor* pActor, sead::Vector3f* pRotate, const char* pJointName);
void initJointLocalXRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName);
void initJointLocalYRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName);
void initJointLocalZRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName);
void initJointLocalAxisRotator(const LiveActor* pActor, const sead::Vector3f& rAxis, f32* pDegree,
                               const char* pJointName);
JointLocalAxisRotator* initJointLocalAxisRotator_RS(const LiveActor* pActor, const sead::Vector3f& rAxis,
                                  f32* pDegree, const char* pJointName, bool isLocal);
void initJointGlobalXRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName);
void initJointGlobalAxisRotator(const LiveActor* pActor, const sead::Vector3f& rAxis, f32* pDegree,
                                const char* pJointName);
void initJointGlobalYRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName);
void initJointGlobalZRotator(const LiveActor* pActor, f32* pDegree, const char* pJointName);
void initJointLocalTransController(const LiveActor* pActor, const sead::Vector3f* pTrans,
                                   const char* pJointName);
void initJointLocalMtxController(const LiveActor* pActor, const sead::Matrix34f* pMtx,
                                 const char* pJointName);
void initJointGlobalMtxController(const LiveActor* pActor, const sead::Matrix34f* pMtx,
                                  const char* pJointName);
void initJointGlobalQuatController(const LiveActor* pActor, const sead::Quatf* pQuat,
                                   const char* pJointName);
void initJointLocalDirController(const LiveActor* pActor, const JointDirectionInfo* pInfo,
                                 const char* pJointName);
void initJointAimController(const LiveActor* pActor, const JointAimInfo* pInfo,
                            const char* pJointName);
JointTranslateShaker* initJointTranslateShaker(const LiveActor* pActor, s32 maxJoints);
void appendJointTranslateShakerX(JointTranslateShaker* pShaker, const char* pJointName);
void appendJointTranslateShakerY(JointTranslateShaker* pShaker, const char* pJointName);
void appendJointTranslateShakerZ(JointTranslateShaker* pShaker, const char* pJointName);
JointMasher* initJointMasher(const LiveActor* pActor, const bool* pIsValid, s32 maxJoints);
void appendMashJoint(JointMasher* pMasher, const char* pJointName, f32 rate);
JointRumbler* initJointRumbler(const LiveActor* pActor, const char* pJointName, f32 cycle, f32 power,
                      u32 duration, s32 startStep);
JointLocalQuatRotator* initJointLocalQuatRotator(const LiveActor* pActor, const char* pJointName,
                               const sead::Quatf* pQuat);
JointLookAtController* initJointLookAtController(const LiveActor* pActor, s32 maxJoints);
void appendJointLookAtController(JointLookAtController* pController, const LiveActor* pActor,
                                 const char* pJointName, f32 rate, const sead::Vector2f& rYawRange,
                                 const sead::Vector2f& rPitchRange, const sead::Vector3f& rLocalFront,
                                 const sead::Vector3f& rLocalUp);
void appendJointLookAtControllerNoJudge(JointLookAtController* pController,
                                        const LiveActor* pActor, const char* pJointName, f32 rate,
                                        const sead::Vector2f& rYawRange,
                                        const sead::Vector2f& rPitchRange,
                                        const sead::Vector3f& rLocalFront,
                                        const sead::Vector3f& rLocalUp);
JointSpringController* initJointSpringController(const LiveActor* pActor, const char* pJointName);
void pauseJointControllers();
void resumeJointControllers();
bool isPausedJointControllers();

}  // namespace al
