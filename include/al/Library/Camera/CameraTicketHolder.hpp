#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraTicket;
class PlacementId;

class CameraTicketHolder {
public:
    CameraTicketHolder(s32 maxTickets);

    void endInit();
    void registerTicket(CameraTicket* pTicket);
    void registerDefaultTicket(CameraTicket* pTicket);
    CameraTicket* tryFindEntranceTicket(const PlacementId* pPlacementId, const char* pSuffix) const;

private:
    CameraTicket** mTickets;
    s32 mTicketNum = 0;
    s32 mMaxTickets;
    CameraTicket* mDefaultTicket = nullptr;
};

}  // namespace al
