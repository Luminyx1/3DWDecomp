#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoser_RS;

using CameraParamTransferFunc = void (*)(CameraPoser_RS* pPrev, CameraPoser_RS* pNext);

struct NameToCameraParamTransferFunc {
    const char* prevName;
    const char* nextName;
    CameraParamTransferFunc func;
};

class CameraParamTransfer {
public:
    CameraParamTransfer();

    void setFuncTable(const NameToCameraParamTransferFunc* pTable, s32 size);
    bool tryTransferParam(CameraPoser_RS* pPrev, CameraPoser_RS* pNext) const;
    CameraParamTransferFunc tryFindTransferFunc(const char* pPrevName, const char* pNextName) const;

private:
    const NameToCameraParamTransferFunc* mFuncTable = nullptr;
    s32 mFuncTableSize = 0;
};

}  // namespace al
