#include "Library/Camera/CameraParamTransfer.hpp"

#include "Library/Camera/CameraPoser_RS.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Creates a transfer without a function table.
 */
CameraParamTransfer::CameraParamTransfer() = default;

/**
 * Sets the table of parameter transfer functions.
 * @param pTable Function table.
 * @param size Number of entries.
 */
void CameraParamTransfer::setFuncTable(const NameToCameraParamTransferFunc* pTable, s32 size) {
    mFuncTable = pTable;
    mFuncTableSize = size;
}

/**
 * Transfers parameters from the previous to the next camera if a function is registered.
 * @param pPrev Previous camera.
 * @param pNext Next camera.
 * @return Whether parameters were transferred.
 */
bool CameraParamTransfer::tryTransferParam(CameraPoser_RS* pPrev, CameraPoser_RS* pNext) const {
    CameraParamTransferFunc func = tryFindTransferFunc(pPrev->getName(), pNext->getName());

    if (!func) {
        return false;
    }

    func(pPrev, pNext);
    return true;
}

/**
 * Finds the transfer function for a pair of camera names.
 * @param pPrevName Name of the previous camera.
 * @param pNextName Name of the next camera.
 * @return Transfer function or null.
 */
CameraParamTransferFunc CameraParamTransfer::tryFindTransferFunc(const char* pPrevName,
                                                                 const char* pNextName) const {
    if (!mFuncTable) {
        return nullptr;
    }

    for (s32 i = 0; i < mFuncTableSize; i++) {
        if (isEqualString(pPrevName, mFuncTable[i].prevName) &&
            isEqualString(pNextName, mFuncTable[i].nextName)) {
            return mFuncTable[i].func;
        }
    }

    return nullptr;
}

}  // namespace al
