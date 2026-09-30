#pragma once

namespace al {
class ByamlIter;
class PlacementId;

class CameraTicketId {
public:
    CameraTicketId(const PlacementId* pPlacementId, const char* pSuffix);

    bool isEqual(const CameraTicketId& rOther) const;
    static bool isEqual(const CameraTicketId& rTicket1, const CameraTicketId& rTicket2);
    bool isEqual(const ByamlIter& rIter) const;
    static bool isEqual(const ByamlIter& rIter, const PlacementId* pPlacementId);
    const char* tryGetObjId() const;
    const char* getObjId() const;

    const PlacementId* getPlacementId() const { return mPlacementId; }

    const char* getSuffix() const { return mSuffix; }

private:
    const PlacementId* mPlacementId;
    const char* mSuffix;
};

}  // namespace al
