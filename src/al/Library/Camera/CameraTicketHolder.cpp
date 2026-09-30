#include "Library/Camera/CameraTicketHolder.hpp"

#include "Library/Camera/CameraPoser_RS.hpp"
#include "Library/Camera/CameraTicket.hpp"
#include "Library/Camera/CameraTicketId.hpp"

namespace al {

CameraTicketHolder::CameraTicketHolder(s32 maxTickets) : mMaxTickets(maxTickets) {
    mTickets = new CameraTicket*[maxTickets];

    for (s32 i = 0; i < mMaxTickets; i++) {
        mTickets[i] = nullptr;
    }
}

void CameraTicketHolder::endInit() {
    for (s32 i = 0; i < mTicketNum; i++) {
        mTickets[i]->getPoser()->endInit();
    }

    if (mDefaultTicket) {
        mDefaultTicket->getPoser()->endInit();
    }
}

void CameraTicketHolder::registerTicket(CameraTicket* pTicket) {
    if (pTicket->getPriority() == CameraTicket::Priority_Default) {
        registerDefaultTicket(pTicket);
        return;
    }

    mTickets[mTicketNum] = pTicket;
    mTicketNum++;
}

void CameraTicketHolder::registerDefaultTicket(CameraTicket* pTicket) {
    mDefaultTicket = pTicket;
}

CameraTicket* CameraTicketHolder::tryFindEntranceTicket(const PlacementId* pPlacementId,
                                                        const char* pSuffix) const {
    CameraTicketId ticketId(pPlacementId, pSuffix);

    for (s32 i = 0; i < mTicketNum; i++) {
        if (mTickets[i]->getPriority() == CameraTicket::Priority_Entrance &&
            mTickets[i]->getTicketId()->isEqual(ticketId)) {
            return mTickets[i];
        }
    }

    return nullptr;
}

}  // namespace al
