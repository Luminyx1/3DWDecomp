#pragma once

#include <type_traits>

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>

namespace al {
class ActorInitInfo;
class AreaInitInfo;
class ByamlIter;
class LiveActor;
class PlacementId;
struct PlacementInfo;

SEAD_ENUM(LayerId, Common, CommonSub, NoUse, Disaster, Scenario1, Scenario2, Scenario3, ScenarioX, Scenario3_X, NoScenario1, NoScenario2, NoScenario3, NoScenarioX, Phase1Add, Phase1Only, Phase1NotChase, Phase2Add, Phase2Only, Phase2NotChase, Phase3Add, Phase3Only, Phase34Chase, Phase34NotChase, Phase4Add, Phase4Only, NoLayer)

bool isValidInfo(const PlacementInfo& rInfo);
bool isPlaced(const ActorInitInfo& rInfo);
void getObjectName(const char** pName, const ActorInitInfo& rInfo);
void getObjectName(const char** pName, const PlacementInfo& rInfo);
bool tryGetObjectName(const char** pName, const PlacementInfo& rInfo);
bool tryGetObjectName(const char** pName, const ActorInitInfo& rInfo);
bool tryGetStringArg(const char** pArg, const PlacementInfo& rInfo, const char* pKey);
bool isObjectName(const ActorInitInfo& rInfo, const char* pName);
bool isObjectName(const PlacementInfo& rInfo, const char* pName);
bool isObjectNameSubStr(const ActorInitInfo& rInfo, const char* pName);
bool isObjectNameSubStr(const PlacementInfo& rInfo, const char* pName);
bool tryGetClassName(const char** pName, const ActorInitInfo& rInfo);
bool tryGetClassName(const char** pName, const PlacementInfo& rInfo);
bool tryGetPlacementInfoByKey(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pKey);
void getClassName(const char** pName, const ActorInitInfo& rInfo);
void getClassName(const char** pName, const PlacementInfo& rInfo);
bool isClassName(const ActorInitInfo& rInfo, const char* pName);
bool isClassName(const PlacementInfo& rInfo, const char* pName);
void getDisplayName(const char** pName, const ActorInitInfo& rInfo);
bool tryGetDisplayName(const char** pName, const ActorInitInfo& rInfo);
void getDisplayName(const char** pName, const PlacementInfo& rInfo);
bool tryGetDisplayName(const char** pName, const PlacementInfo& rInfo);
void getPlacementTargetFile(const char** pFile, const PlacementInfo& rInfo);
bool tryGetTrans(sead::Vector3f* pTrans, const ActorInitInfo& rInfo);
bool tryGetTrans(sead::Vector3f* pTrans, const PlacementInfo& rInfo);
void multZoneMtx(sead::Vector3f* pTrans, const PlacementInfo& rInfo);
void getTrans(sead::Vector3f* pTrans, const PlacementInfo& rInfo);
bool tryGetRotate(sead::Vector3f* pRotate, const ActorInitInfo& rInfo);
bool tryGetRotate(sead::Vector3f* pRotate, const PlacementInfo& rInfo);
bool tryGetZoneMatrixTR(sead::Matrix34f* pMtx, const PlacementInfo& rInfo);
bool tryGetRotate_ParentY(sead::Vector3f* pRotate, const ActorInitInfo& rInfo);
bool tryGetRotate_ParentY(sead::Vector3f* pRotate, const PlacementInfo& rInfo);
bool tryGetZoneR(sead::Vector3f* pRotate, const PlacementInfo& rInfo);
void getRotate(sead::Vector3f* pRotate, const PlacementInfo& rInfo);
bool tryGetQuat(sead::Quatf* pQuat, const ActorInitInfo& rInfo);
bool tryGetQuat(sead::Quatf* pQuat, const PlacementInfo& rInfo);
void getQuat(sead::Quatf* pQuat, const PlacementInfo& rInfo);
bool tryGetScale(sead::Vector3f* pScale, const ActorInitInfo& rInfo);
bool tryGetScale(sead::Vector3f* pScale, const PlacementInfo& rInfo);
bool tryGetSide(sead::Vector3f* pSide, const ActorInitInfo& rInfo);
bool tryGetSide(sead::Vector3f* pSide, const PlacementInfo& rInfo);
bool tryGetUp(sead::Vector3f* pUp, const ActorInitInfo& rInfo);
bool tryGetUp(sead::Vector3f* pUp, const PlacementInfo& rInfo);
bool tryGetFront(sead::Vector3f* pFront, const ActorInitInfo& rInfo);
bool tryGetFront(sead::Vector3f* pFront, const PlacementInfo& rInfo);
bool tryGetLocalAxis(sead::Vector3f* pDir, const ActorInitInfo& rInfo, s32 axis);
bool tryGetLocalAxis(sead::Vector3f* pDir, const PlacementInfo& rInfo, s32 axis);
bool tryGetLocalSignAxis(sead::Vector3f* pDir, const ActorInitInfo& rInfo, s32 axis);
bool tryGetLocalSignAxis(sead::Vector3f* pDir, const PlacementInfo& rInfo, s32 axis);
bool tryGetMatrixTR(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo);
bool tryGetMatrixTR(sead::Matrix34f* pMtx, const PlacementInfo& rInfo);
bool tryGetMatrixTRS(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo);
bool tryGetMatrixTRS(sead::Matrix34f* pMtx, const PlacementInfo& rInfo);
bool tryGetInvertMatrixTR(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo);
bool tryGetInvertMatrixTR(sead::Matrix34f* pMtx, const PlacementInfo& rInfo);
void calcMatrixMultParent(sead::Matrix34f* pMtx, const PlacementInfo& rInfo,
                          const PlacementInfo& rParentInfo);
void calcMatrixMultParent(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo,
                          const ActorInitInfo& rParentInfo);
bool tryGetArg(s32* pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetArg(s32* pArg, const PlacementInfo& rInfo, const char* pKey);

/// Reads an integer argument straight into an enum stored as s32.
template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool tryGetArg(T* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    static_assert(sizeof(T) == sizeof(s32));
    return tryGetArg(reinterpret_cast<s32*>(pArg), rInfo, pKey);
}

/// Reads an integer argument straight into an enum stored as s32.
template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool tryGetArg(T* pArg, const PlacementInfo& rInfo, const char* pKey) {
    static_assert(sizeof(T) == sizeof(s32));
    return tryGetArg(reinterpret_cast<s32*>(pArg), rInfo, pKey);
}
bool tryGetArg(f32* pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetArg(f32* pArg, const PlacementInfo& rInfo, const char* pKey);
bool tryGetArg(bool* pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetArg(bool* pArg, const PlacementInfo& rInfo, const char* pKey);
void getArg(s32* pArg, const ActorInitInfo& rInfo, const char* pKey);
void getArg(s32* pArg, const PlacementInfo& rInfo, const char* pKey);
void getArg(f32* pArg, const ActorInitInfo& rInfo, const char* pKey);
void getArg(f32* pArg, const PlacementInfo& rInfo, const char* pKey);
void getArg(bool* pArg, const ActorInitInfo& rInfo, const char* pKey);
void getArg(bool* pArg, const PlacementInfo& rInfo, const char* pKey);
void getStringArg(const char** pArg, const ActorInitInfo& rInfo, const char* pKey);
void getStringArg(const char** pArg, const PlacementInfo& rInfo, const char* pKey);
void getStringArg(const char** pArg, const AreaInitInfo& rInfo, const char* pKey);
bool tryGetStringArg(const char** pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetStringArg(const char** pArg, const AreaInitInfo& rInfo, const char* pKey);
bool tryGetArgV2f(sead::Vector2f* pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetArgV2f(sead::Vector2f* pArg, const PlacementInfo& rInfo, const char* pKey);
bool tryGetArgV3f(sead::Vector3f* pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetArgV3f(sead::Vector3f* pArg, const PlacementInfo& rInfo, const char* pKey);
bool tryGetArgColor(sead::Color4f* pArg, const ActorInitInfo& rInfo, const char* pKey);
bool tryGetArgColor(sead::Color4f* pArg, const PlacementInfo& rInfo, const char* pKey);
s32 getCountPlacementInfo(const PlacementInfo& rInfo);
void getPlacementInfoByKey(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pKey);
void getPlacementInfoByIndex(PlacementInfo* pOut, const PlacementInfo& rInfo, s32 index);
bool tryGetPlacementInfoByIndex(PlacementInfo* pOut, const PlacementInfo& rInfo, s32 index);
void getPlacementInfoAndKeyNameByIndex(PlacementInfo* pOut, const char** pKey,
                                       const PlacementInfo& rInfo, s32 index);
bool tryGetPlacementInfoAndKeyNameByIndex(PlacementInfo* pOut, const char** pKey,
                                          const PlacementInfo& rInfo, s32 index);
s32 tryGetLayerID(const ActorInitInfo& rInfo);
s32 tryGetLayerID(const PlacementInfo& rInfo);
s32 tryGetLayerID(const ByamlIter& rIter);
s32 tryGetLayerIDbyParents(const PlacementInfo& rInfo);
bool tryGetPlacementID(PlacementId* pId, const ActorInitInfo& rInfo);
bool tryGetPlacementID(PlacementId* pId, const PlacementInfo& rInfo);
void getPlacementId(PlacementId* pId, const ActorInitInfo& rInfo);
void getPlacementId(PlacementId* pId, const PlacementInfo& rInfo);
bool isEqualPlacementID(const PlacementId& rId, const PlacementId& rOther);
bool isEqualPlacementID(const PlacementInfo& rInfo, const PlacementInfo& rOther);
bool isExistRail(const ActorInitInfo& rInfo);
bool tryGetRailIter(PlacementInfo* pRailInfo, const PlacementInfo& rInfo);
bool tryGetLinksInfo(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pLinkName);
bool tryGetMoveParameterRailIter(PlacementInfo* pRailInfo, const PlacementInfo& rInfo);
bool tryGetRailPointPos(sead::Vector3f* pPos, const PlacementInfo& rInfo);
void getRailPointHandlePrev(sead::Vector3f* pPos, const PlacementInfo& rInfo);
bool tryGetRailPointHandlePrev(sead::Vector3f* pPos, const PlacementInfo& rInfo);
void getRailPointHandleNext(sead::Vector3f* pPos, const PlacementInfo& rInfo);
bool tryGetRailPointHandleNext(sead::Vector3f* pPos, const PlacementInfo& rInfo);
s32 calcLinkChildNum(const ActorInitInfo& rInfo, const char* pLinkName);
s32 calcLinkChildNum(const PlacementInfo& rInfo, const char* pLinkName);
s32 calcLinkNestNum(const ActorInitInfo& rInfo, const char* pLinkName);
s32 calcLinkNestNum(const PlacementInfo& rInfo, const char* pLinkName);
void getLinksInfo(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pLinkName);
void getLinksInfoByIndex(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pLinkName,
                         s32 index);
void getLinksInfoByIndex(PlacementInfo* pOut, const ActorInitInfo& rInfo, const char* pLinkName,
                         s32 index);
bool tryGetLinksInfo(PlacementInfo* pOut, const ActorInitInfo& rInfo, const char* pLinkName);
void getLinksMatrix(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo, const char* pLinkName);
void getLinksMatrixByIndex(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo,
                           const char* pLinkName, s32 index);
void getLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const PlacementInfo& rInfo,
               const char* pLinkName);
void getLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const ActorInitInfo& rInfo,
               const char* pLinkName);
void getLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const AreaInitInfo& rInfo,
               const char* pLinkName);
bool tryGetLinksQT(sead::Quatf* pQuat, sead::Vector3f* pTrans, const ActorInitInfo& rInfo,
                   const char* pLinkName);
bool tryGetLinksQTS(sead::Quatf* pQuat, sead::Vector3f* pTrans, sead::Vector3f* pScale,
                    const ActorInitInfo& rInfo, const char* pLinkName);
bool tryGetLinksMatrixTRS(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo,
                          const char* pLinkName);
bool tryGetLinksTrans(sead::Vector3f* pTrans, const ActorInitInfo& rInfo, const char* pLinkName);
bool tryGetLinksQuat(sead::Quatf* pQuat, const ActorInitInfo& rInfo, const char* pLinkName);
void getChildLinkT(sead::Vector3f* pTrans, const ActorInitInfo& rInfo, const char* pLinkName,
                   s32 index);
void getChildLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const ActorInitInfo& rInfo,
                    const char* pLinkName, s32 index);
void getChildLinkTF(sead::Vector3f* pTrans, sead::Vector3f* pFront, const ActorInitInfo& rInfo,
                    const char* pLinkName, s32 index);
void getChildLinkTQ(sead::Vector3f* pTrans, sead::Quatf* pQuat, const ActorInitInfo& rInfo,
                    const char* pLinkName, s32 index);
s32 calcMatchNameLinkCount(const PlacementInfo& rInfo, const char* pMatch);
s32 calcLinkCountClassName(const PlacementInfo& rInfo, const char* pClassName);
bool tryGetZoneMatrixTR(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo);
bool tryGetZoneID(s32* pId, const PlacementInfo& rInfo);
bool tryGetDisplayOffset(sead::Vector3f* pOffset, const ActorInitInfo& rInfo);
bool tryGetDisplayScale(sead::Vector3f* pScale, const ActorInitInfo& rInfo);
bool isSingleMode(const ActorInitInfo& rInfo);
bool isExistLinkChild(const PlacementInfo& rInfo, const char* pLinkName, s32 index);

const char* getLinksActorDisplayName(const ActorInitInfo&, const char*, s32);
void initLinksActor(LiveActor*, const ActorInitInfo&, const char*, s32);
}  // namespace al

namespace alPlacementFunction {
s32 getCameraId(const al::ActorInitInfo& rInfo);
bool getLinkGroupId(al::PlacementId* pId, const al::ActorInitInfo& rInfo, const char* pLinkName);
bool isEnableLinkGroupId(const al::ActorInitInfo& rInfo, const char* pLinkName);
bool isEnableGroupClipping(const al::ActorInitInfo& rInfo);
bool getClippingGroupId(al::PlacementId* pId, const al::ActorInitInfo& rInfo);
bool getClippingViewId(al::PlacementId* pId, const al::PlacementInfo& rInfo);
bool getClippingViewId(al::PlacementId* pId, const al::ActorInitInfo& rInfo);
void getModelName(const char** pName, const al::ActorInitInfo& rInfo);
void getModelName(const char** pName, const al::PlacementInfo& rInfo);
bool tryGetModelName(const char** pName, const al::PlacementInfo& rInfo);
bool tryGetModelName(const char** pName, const al::ActorInitInfo& rInfo);
}  // namespace alPlacementFunction
