#include "Library/Camera/CameraTicketId.hpp"

#include "Library/Camera/CameraTicket.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
inline bool isEqualStringOrBothNull(const char* pStr1, const char* pStr2) {
    if (pStr1 != nullptr && pStr2 != nullptr) {
        return isEqualString(pStr1, pStr2);
    }

    return pStr1 == nullptr && pStr2 == nullptr;
}
}  // namespace

/**
 * Constructs a camera ticket id from a placement id and a suffix.
 * @param pPlacementId placement id of the owning object (may be null)
 * @param pSuffix camera name suffix (may be null)
 */
CameraTicketId::CameraTicketId(const PlacementId* pPlacementId, const char* pSuffix)
    : mPlacementId(pPlacementId), mSuffix(pSuffix) {}

/**
 * Checks whether this id refers to the same camera as another id.
 * @param rOther id to compare with
 * @return true if placement ids and suffixes are equal
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

    bool isNoneSuffix = mSuffix == nullptr && (rOther.mSuffix == nullptr);

    if (mSuffix != nullptr && rOther.mSuffix != nullptr) {
        return isEqualString(mSuffix, rOther.mSuffix);
    }

    return isNoneSuffix;
}

/**
 * Checks whether two ids refer to the same camera.
 * @param rTicket1 first id
 * @param rTicket2 second id
 * @return true if both ids are equal
 */
bool CameraTicketId::isEqual(const CameraTicketId& rTicket1, const CameraTicketId& rTicket2) {
    return rTicket1.isEqual(rTicket2);
}

/**
 * Checks whether this id matches the ObjId and Suffix entries of a camera parameter.
 * @param rIter camera parameter iterator
 * @return true if ObjId and Suffix match
 */
bool CameraTicketId::isEqual(const ByamlIter& rIter) const {
    if (!isEqualStringOrBothNull(getObjId(), tryGetByamlKeyStringOrNULL(rIter, "ObjId"))) {
        return false;
    }

    return isEqualStringOrBothNull(mSuffix, tryGetByamlKeyStringOrNULL(rIter, "Suffix"));
}

/**
 * Checks whether a camera parameter belongs to a placement id and has no suffix.
 * @param rIter camera parameter iterator
 * @param pPlacementId placement id to compare with
 * @return true if ObjId matches and no Suffix is set
 */
bool CameraTicketId::isEqual(const ByamlIter& rIter, const PlacementId* pPlacementId) {
    if (!isEqualStringOrBothNull(pPlacementId->mPlacementID, tryGetByamlKeyStringOrNULL(rIter, "ObjId"))) {
        return false;
    }

    return tryGetByamlKeyStringOrNULL(rIter, "Suffix") == nullptr;
}

/**
 * Gets the object id of the placement this id belongs to.
 * @return object id, or null without a placement id
 */
const char* CameraTicketId::tryGetObjId() const {
    if (mPlacementId == nullptr) {
        return nullptr;
    }

    return mPlacementId->mPlacementID;
}

/**
 * Gets the object id of the placement this id belongs to.
 * @return object id, or null without a placement id
 */
const char* CameraTicketId::getObjId() const {
    return tryGetObjId();
}

/**
 * Constructs a camera ticket.
 * @param pPoser camera poser started by this ticket
 * @param pTicketId id of the camera
 * @param priority camera priority
 */
CameraTicket::CameraTicket(CameraPoser_RS* pPoser, const CameraTicketId* pTicketId, s32 priority)
    : mPoser(pPoser), mTicketId(pTicketId), mPriority(priority) {}

/**
 * Sets the camera priority.
 * @param priority new priority
 */
void CameraTicket::setPriority(s32 priority) {
    mPriority = priority;
}

}  // namespace al
