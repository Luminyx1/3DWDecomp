#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class LiveActor;
}  // namespace al

/** @brief A checkpoint (warp object) record stored in recorded ghost play data. */
struct GhostWarpObjData {
    char mName[20];
    u16 mFrame;
    bool mIsRestartPoint;
};

namespace GhostPlayerFunction {
u32 getGhostPlayerRecorderBufferSize();
void makeGhostActionName(sead::BufferedSafeString* pOut, const char* pActionName,
                         const al::LiveActor* pActor);
bool isValidGhostPlayerDataVersion(const void* pData);
const GhostWarpObjData* getWarpObjData(const void* pData, s32 index);
s32 getWarpObjDataNum(const void* pData);
s32 getGhostPlayDataNum(const void* pData);
void calcGhostPlayDataTrans(sead::Vector3f* pTrans, const void* pData, s32 frame, f32 maxDist);
void calcGhostPlayDataRotate(sead::Vector3f* pRotate, const void* pData, s32 frame);
void calcGhostPlayDataSklAnimFrame(f32* pFrame, const void* pData, s32 frame, bool isOneTime);
void calcGhostPlayDataActionName(const char** pName, const void* pData, s32 frame);
}  // namespace GhostPlayerFunction
