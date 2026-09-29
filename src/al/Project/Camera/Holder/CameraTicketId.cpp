#include "Project/Camera/Holder/CameraTicketId.hpp"

#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Holder/CameraTicket.hpp"

namespace al {
namespace {
/**
 * @brief Compares two strings that may be missing.
 * @param pStr1 The first string, or null.
 * @param pStr2 The second string, or null.
 * @return True if both strings are equal or both are missing.
 */
inline bool isEqualStringOrBothNull(const char* pStr1, const char* pStr2) {
    if (pStr1 != nullptr && pStr2 != nullptr) {
        return isEqualString(pStr1, pStr2);
    }
    return pStr1 == nullptr && pStr2 == nullptr;
}
}  // namespace

/**
 * @brief Creates an id from an owner placement and a suffix.
 * @param pPlacementId The placement id of the camera's owner, or null for cameras without an owner.
 * @param pSuffix The suffix telling apart the cameras of one owner, or null.
 */
CameraTicketId::CameraTicketId(const PlacementId* pPlacementId, const char* pSuffix)
    : mPlacementId(pPlacementId), mSuffix(pSuffix) {}

/**
 * @brief Checks whether two ids name the same camera.
 * @param rOther The other id.
 * @return True if both placement ids and suffixes are equal or both missing.
 */
bool CameraTicketId::isEqual(const CameraTicketId& rOther) const {
    if (mPlacementId == nullptr && rOther.mPlacementId != nullptr) {
        return false;
    }
    if (mPlacementId != nullptr && rOther.mPlacementId == nullptr) {
        return false;
    }

    if (mPlacementId == nullptr && rOther.mPlacementId == nullptr) {
        return isEqualString(mSuffix, rOther.mSuffix);
    }
    if (!mPlacementId->isEqual(*rOther.mPlacementId)) {
        return false;
    }

    bool isNoneSuffix = mSuffix == nullptr && rOther.mSuffix == nullptr;
    if (mSuffix != nullptr && rOther.mSuffix != nullptr) {
        return isEqualString(mSuffix, rOther.mSuffix);
    }
    return isNoneSuffix;
}

/**
 * @brief Checks whether two ids name the same camera.
 * @param rTicketId1 The first id.
 * @param rTicketId2 The second id.
 * @return True if both ids are equal.
 */
bool CameraTicketId::isEqual(const CameraTicketId& rTicketId1, const CameraTicketId& rTicketId2) {
    return rTicketId1.isEqual(rTicketId2);
}

/**
 * @brief Checks whether the id matches the "ObjId" and "Suffix" given in a parameter file.
 * @param rIter The parameters.
 * @return True if the object id and suffix are equal or both missing.
 */
bool CameraTicketId::isEqual(const ByamlIter& rIter) const {
    if (!isEqualStringOrBothNull(getObjId(), tryGetByamlKeyStringOrNULL(rIter, "ObjId"))) {
        return false;
    }
    return isEqualStringOrBothNull(mSuffix, tryGetByamlKeyStringOrNULL(rIter, "Suffix"));
}

/**
 * @brief Gets the placement id string of the owner.
 * @return The id string, or null if the camera has no owner.
 */
const char* CameraTicketId::tryGetObjId() const {
    if (mPlacementId == nullptr) {
        return nullptr;
    }
    return mPlacementId->mPlacementID;
}

/**
 * @brief Checks whether the "ObjId" of a parameter file names a placement and it has no "Suffix".
 * @param rIter The parameters.
 * @param pPlacementId The placement id to compare with.
 * @return True if the object id matches and no suffix is given.
 */
bool CameraTicketId::isEqual(const ByamlIter& rIter, const PlacementId* pPlacementId) {
    if (!isEqualStringOrBothNull(pPlacementId->mPlacementID, tryGetByamlKeyStringOrNULL(rIter, "ObjId"))) {
        return false;
    }
    return tryGetByamlKeyStringOrNULL(rIter, "Suffix") == nullptr;
}

/**
 * @brief Gets the placement id string of the owner.
 * @return The id string, or null if the camera has no owner.
 */
const char* CameraTicketId::getObjId() const {
    return tryGetObjId();
}

/**
 * @brief Creates a ticket for a camera poser.
 * @param pPoser The poser of the camera.
 * @param pTicketId The id of the camera.
 * @param priority The priority of the camera.
 */
CameraTicket::CameraTicket(CameraPoser_RS* pPoser, const CameraTicketId* pTicketId, s32 priority)
    : mPoser(pPoser), mTicketId(pTicketId), mPriority(priority) {}

/**
 * @brief Changes the priority of the camera.
 * @param priority The new priority.
 */
void CameraTicket::setPriority(s32 priority) {
    mPriority = priority;
}
}  // namespace al
