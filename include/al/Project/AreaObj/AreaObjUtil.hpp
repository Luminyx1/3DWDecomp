#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class AreaObj;
class AreaObjGroup;
class CameraDirector_RS;
class IUseAreaObj;
class LiveActor;
class PlayerHolder;

class AreaObjFilterBase {
public:
    virtual bool isValidArea(AreaObj* pAreaObj) const = 0;
};

AreaObj* tryFindAreaObj(const IUseAreaObj* pAreaUser, const char* pName,
                        const sead::Vector3f& rPos);
AreaObj* tryFindAreaObj(const IUseAreaObj* pAreaUser, const char* pName,
                        const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                        sead::Vector3f* pHitPos, sead::Vector3f* pNormal);
AreaObj* tryFindAreaObjWithFilter(const IUseAreaObj* pAreaUser, const char* pName,
                                  const sead::Vector3f& rPos, AreaObjFilterBase* pFilter);
AreaObjGroup* tryFindAreaObjGroup(const IUseAreaObj* pAreaUser, const char* pName);
AreaObj* tryFindAreaObj(const IUseAreaObj* pAreaUser, const char* pName);
AreaObj* tryFindPlessieAreaObj(const IUseAreaObj* pAreaUser, const char* pName);
AreaObj* tryFindPlessieTunnelAreaObj(const IUseAreaObj* pAreaUser, const char* pName);
bool isInAreaObjInGroup(const IUseAreaObj* pAreaUser, const char* pName,
                        const sead::Vector3f& rPos);
bool isInDisasterCameraArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
bool isInPlessieCameraArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
AreaObj* getStartCameraArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos,
                            CameraDirector_RS* pCameraDirector);
void addToExtraAreaObjectGroup(const IUseAreaObj* pAreaUser, AreaObj* pAreaObj);
AreaObj* tryGetAreaInExtraAreaGroupAtPos(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
AreaObj* tryFindAreaObjPlayerOne(const IUseAreaObj* pAreaUser, const char* pName,
                                 const PlayerHolder* pPlayerHolder);
AreaObj* tryFindAreaObjPlayerAll(const IUseAreaObj* pAreaUser, const char* pName,
                                 const PlayerHolder* pPlayerHolder);
bool isInAreaObj(AreaObjGroup* pGroup, const sead::Vector3f& rPos);
bool tryIsInAreaObj(AreaObjGroup* pGroup, const sead::Vector3f& rPos);
AreaObj* tryGetAreaObj(AreaObjGroup* pGroup, const sead::Vector3f& rPos);

bool isInAreaPos(const AreaObj* pAreaObj, const sead::Vector3f& rPos);
bool isInAreaObj(const IUseAreaObj* pAreaUser, const char* pName, const sead::Vector3f& rPos);
bool isInAreaObj(const IUseAreaObj* pAreaUser, const char* pName, const sead::Vector3f& rStart,
                 const sead::Vector3f& rEnd, sead::Vector3f* pHitPos, sead::Vector3f* pNormal);
bool isInAreaObj(const LiveActor* pActor, const char* pName);
bool isInAreaObjPlayerOne(const IUseAreaObj* pAreaUser, const char* pName,
                          const PlayerHolder* pPlayerHolder);
bool isInAreaObjPlayerAll(const IUseAreaObj* pAreaUser, const char* pName,
                          const PlayerHolder* pPlayerHolder);
bool isExistAreaObj(const IUseAreaObj* pAreaUser, const char* pName);
bool isInDeathArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);

bool isInWaterArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
bool isInWaterArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rStart,
                   const sead::Vector3f& rEnd, sead::Vector3f* pHitPos, sead::Vector3f* pNormal);

bool isInPlayerControlOffArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);

f32 calcWaterSinkDepth(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
bool calcWaterDistanceCheck(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos,
                            f32 distance, f32* pHeight);
f32 calcWaterSinkDepth(const LiveActor* pActor);
bool isInPlessieTunnel(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
bool tryGetAreaObjArg(s32* pArg, const AreaObj* pAreaObj, const char* pKey);
bool tryGetAreaObjArg(f32* pArg, const AreaObj* pAreaObj, const char* pKey);
bool tryGetAreaObjArg(bool* pArg, const AreaObj* pAreaObj, const char* pKey);
bool tryGetAreaObjStringArg(const char** pArg, const AreaObj* pAreaObj, const char* pKey);
bool tryIsInAreaObjPlayer(AreaObjGroup* pGroup);
AreaObj* tryGetAreaObjPlayer(AreaObjGroup* pGroup);
bool tryIsInAreaPlayer(const AreaObj* pAreaObj);
const sead::Matrix34f& getAreaObjBaseMtx(const AreaObj* pAreaObj);

void calcNearestAreaObjEdgePos(sead::Vector3f* pOut, const AreaObj* pAreaObj,
                               const sead::Vector3f& rPos);
bool checkAreaObjCollisionByArrow(sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                                  const AreaObj* pAreaObj, const sead::Vector3f& rStart,
                                  const sead::Vector3f& rEnd);
AreaObj* tryFindAreaObjByName(const IUseAreaObj* pAreaUser, const char* pGroupName,
                              const char* pName);

void registerAreaHostMtx(const LiveActor*, const ActorInitInfo&);
AreaObj* tryFindAreaObjPlayerOne(const IUseAreaObj*, const char*, const PlayerHolder*);
bool isInWaterArea(const LiveActor*);
bool isInWaterArea(const LiveActor*, f32);
}  // namespace al
