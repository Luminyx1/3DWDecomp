#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraTicket;
class PlacementId;

/// Holds the tickets of all cameras of a scene.
class CameraTicketHolder {
public:
    CameraTicketHolder(s32 maxTickets);

    void endInit();
    void registerTicket(CameraTicket* pTicket);
    void registerDefaultTicket(CameraTicket* pTicket);
    CameraTicket* tryFindEntranceTicket(const PlacementId* pPlacementId, const char* pSuffix) const;

    CameraTicket** mTickets;                   // _0
    s32 mNumTickets = 0;                       // _8
    s32 mMaxTickets;                           // _C
    CameraTicket* mDefaultTicket = nullptr;    // _10
};
}  // namespace al
