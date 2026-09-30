#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class GraphicsSystemInfo;
class ModelKeeper;

s32 getJointNum(const ModelKeeper* pKeeper);
s32 getJointIndex(const ModelKeeper* pKeeper, const char* pName);
bool isExistJoint(const ModelKeeper* pKeeper, const char* pName);
const char* getJointName(const ModelKeeper* pKeeper, s32 index);
const sead::Matrix34f* getJointMtxPtr(const ModelKeeper* pKeeper, const char* pName);
const sead::Matrix34f* getJointMtxPtrByIndex(const ModelKeeper* pKeeper, s32 index);
const sead::Matrix34f* getJointLocalMtxPtr(const ModelKeeper* pKeeper, const char* pName);
const void* getJointLocalMtxPtrByIndex(const ModelKeeper* pKeeper, s32 index);
void getJointLocalTrans(sead::Vector3f* pOut, const ModelKeeper* pKeeper, const char* pName);
void getJointLocalTrans(sead::Vector3f* pOut, const ModelKeeper* pKeeper, s32 index);
s32 getParentJointIndex(const ModelKeeper* pKeeper, s32 index);
void setJointVisibility(const ModelKeeper* pKeeper, const char* pName, bool isVisible);
bool getJointVisibility(const ModelKeeper* pKeeper, const char* pName);
s32 getMaterialIndex(const ModelKeeper* pKeeper, const char* pName);
void hideMaterial(ModelKeeper* pKeeper, const char* pName);
void showMaterial(ModelKeeper* pKeeper, const char* pName);
void forceApplyCubeMap(ModelKeeper* pKeeper, const GraphicsSystemInfo* pInfo, const char* pName);
}  // namespace al
