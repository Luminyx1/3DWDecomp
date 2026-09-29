#pragma once

namespace al {
class ByamlIter;
class PlacementId;

/// Identifies a camera by the placement of its owner and a suffix.
class CameraTicketId {
public:
    CameraTicketId(const PlacementId* pPlacementId, const char* pSuffix);

    bool isEqual(const CameraTicketId& rOther) const;
    static bool isEqual(const CameraTicketId& rTicketId1, const CameraTicketId& rTicketId2);
    bool isEqual(const ByamlIter& rIter) const;
    const char* tryGetObjId() const;
    static bool isEqual(const ByamlIter& rIter, const PlacementId* pPlacementId);
    const char* getObjId() const;

    const PlacementId* mPlacementId;  // _0
    const char* mSuffix;              // _8
};
}  // namespace al
