#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoser_RS;

typedef void (*CameraParamTransferFunc)(CameraPoser_RS* pFrom, CameraPoser_RS* pTo);

/// Names a pair of camera types and the function that hands parameters over between them.
struct NameToCameraParamTransferFunc {
    const char* mFromName;          // _0
    const char* mToName;            // _8
    CameraParamTransferFunc mFunc;  // _10
};

/// Hands parameters over from one camera to the next when switching cameras.
class CameraParamTransfer {
public:
    CameraParamTransfer();

    void setFuncTable(const NameToCameraParamTransferFunc* pTable, s32 tableSize);
    bool tryTransferParam(CameraPoser_RS* pFrom, CameraPoser_RS* pTo) const;
    CameraParamTransferFunc tryFindTransferFunc(const char* pFromName, const char* pToName) const;

    const NameToCameraParamTransferFunc* mTable = nullptr;  // _0
    s32 mTableSize = 0;                                     // _8
};
}  // namespace al
