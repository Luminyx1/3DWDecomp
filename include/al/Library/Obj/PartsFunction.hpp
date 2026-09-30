#pragma once

#include <math/seadMatrix.h>

namespace al {
class ActorInitInfo;
class CollisionObj;
class HitSensor;
class LiveActor;
class PartsModel;

CollisionObj* createCollisionObj(const LiveActor* pParent, const ActorInitInfo& rInfo,
                                 const char* pCollisionFileName, HitSensor* pHitSensor,
                                 const char* pJointName, const char* pSuffix);
CollisionObj* createCollisionObjMtx(const LiveActor* pParent, const ActorInitInfo& rInfo,
                                    const char* pCollisionFileName, HitSensor* pHitSensor,
                                    const sead::Matrix34f* pJointMtx, const char* pSuffix);
PartsModel* createPartsModel(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pName,
                             const char* pArchiveName, const sead::Matrix34f* pJointMtx);
PartsModel* createPartsModelFile(LiveActor* pParent, const ActorInitInfo& rInfo,
                                 const char* pName, const char* pArchiveName,
                                 const char* pSuffix);
PartsModel* createPartsModelFileSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                       const char* pName, const char* pArchiveName,
                                       const char* pArchiveSuffix, const char* pSuffix);
PartsModel* createSimplePartsModel(LiveActor* pParent, const ActorInitInfo& rInfo,
                                   const char* pName, const char* pArchiveName,
                                   const char* pSuffix);
PartsModel* createSimplePartsModelSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                         const char* pName, const char* pArchiveName,
                                         const char* pArchiveSuffix, const char* pSuffix);
PartsModel* createPartsModelSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                   const char* pName, const char* pArchiveName,
                                   const char* pSuffix, const sead::Matrix34f* pJointMtx);
PartsModel* createPartsModelJoint(LiveActor* pParent, const ActorInitInfo& rInfo,
                                  const char* pName, const char* pArchiveName,
                                  const char* pJointName);
PartsModel* createPartsModelSuffixJoint(LiveActor* pParent, const ActorInitInfo& rInfo,
                                        const char* pName, const char* pArchiveName,
                                        const char* pArchiveSuffix, const char* pJointName);
void appearBreakModelRandomRotateY(LiveActor* pActor);
bool updateSyncHostVisible(bool* pIsHidden, LiveActor* pActor, const LiveActor* pHost,
                           bool isForceHide);
bool isTraceModelRandomRotate(const LiveActor* pActor);
}  // namespace al
