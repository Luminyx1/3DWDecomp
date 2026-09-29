#include "Project/Camera/Area/CameraParamTransfer.hpp"

#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Main/CameraPoser_RS.hpp"

namespace al {
/** @brief Creates a transfer without any functions. */
CameraParamTransfer::CameraParamTransfer() = default;

/**
 * @brief Sets the table of transfer functions.
 * @param pTable The table.
 * @param tableSize The number of entries in the table.
 */
void CameraParamTransfer::setFuncTable(const NameToCameraParamTransferFunc* pTable, s32 tableSize) {
    mTable = pTable;
    mTableSize = tableSize;
}

/**
 * @brief Hands the parameters of one camera over to the next one if there is a function for their types.
 * @param pFrom The camera that ends.
 * @param pTo The camera that starts.
 * @return True if a transfer function was called.
 */
bool CameraParamTransfer::tryTransferParam(CameraPoser_RS* pFrom, CameraPoser_RS* pTo) const {
    CameraParamTransferFunc func = tryFindTransferFunc(pFrom->getName(), pTo->getName());
    if (func == nullptr) {
        return false;
    }

    func(pFrom, pTo);
    return true;
}

/**
 * @brief Looks for the transfer function between two camera types.
 * @param pFromName The name of the camera that ends.
 * @param pToName The name of the camera that starts.
 * @return The transfer function, or null if there is none.
 */
CameraParamTransferFunc CameraParamTransfer::tryFindTransferFunc(const char* pFromName, const char* pToName) const {
    if (mTable == nullptr) {
        return nullptr;
    }

    for (s32 i = 0; i < mTableSize; i++) {
        if (isEqualString(pFromName, mTable[i].mFromName) && isEqualString(pToName, mTable[i].mToName)) {
            return mTable[i].mFunc;
        }
    }

    return nullptr;
}
}  // namespace al
