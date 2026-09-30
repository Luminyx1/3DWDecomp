#pragma once

#include <prim/seadSafeString.h>

namespace al {
class ByamlIter;
class CameraTicketId;
class PlacementId;
class Resource;

class CameraResourceHolder {
public:
    struct Entry {
        ByamlIter* cameraParam = nullptr;
        ByamlIter* interpoleParam = nullptr;
        sead::FixedSafeString<128> stageName = {""};
        s32 zoneId = -1;
    };

    CameraResourceHolder(const char* pStageName, s32 maxEntries);

    bool tryInitCameraResource(const Resource* pResource, s32 unused);
    void tryInitZoneID(const char* pStageName, s32 zoneId);
    bool tryFindParamResource(ByamlIter* pTicket, const CameraTicketId* pTicketId,
                              s32 paramType) const;
    bool tryFindCameraParamList(ByamlIter* pParamList, const PlacementId* pPlacementId,
                                const char* pParamName) const;
    bool tryFindParamResource(ByamlIter* pTicket, const CameraTicketId* pTicketId, s32 paramType,
                              s32 zoneId) const;
    bool tryFindCameraParamList(ByamlIter* pParamList, const PlacementId* pPlacementId,
                                s32 zoneId, const char* pParamName) const;
    bool tryFindParamResource(ByamlIter* pTicket, const PlacementId* pPlacementId) const;
    s32 calcEntranceCameraParamNum() const;
    bool tryFindCameraParamList(ByamlIter* pParamList, const char* pStageName,
                                const char* pParamName) const;
    s32 calcEntranceCameraParamNum(s32 zoneId) const;
    void getEntranceCameraParamResource(ByamlIter* pTicket, s32 index) const;
    void getEntranceCameraParamResource(ByamlIter* pTicket, s32 index, s32 zoneId) const;
    Entry* findCameraResource(const char* pStageName) const;
    Entry* tryFindCameraResource(const char* pStageName) const;
    Entry* tryFindCameraResource(const PlacementId* pPlacementId) const;

private:
    const char* mStageName;
    s32 mMaxEntries;
    s32 mNumEntries = 0;
    Entry** mEntries;
};

static_assert(sizeof(CameraResourceHolder::Entry) == 0xb0);

}  // namespace al
