#include "Project/Camera/Holder/CameraTicketHolder.hpp"

#include "Project/Camera/Holder/CameraTicket.hpp"
#include "Project/Camera/Holder/CameraTicketId.hpp"
#include "Project/Camera/Main/CameraPoser_RS.hpp"

namespace al {
/**
 * @brief Creates an empty holder.
 * @param maxTickets The maximum number of tickets.
 */
CameraTicketHolder::CameraTicketHolder(s32 maxTickets) : mMaxTickets(maxTickets) {
    mTickets = new CameraTicket*[maxTickets];
    for (s32 i = 0; i < mMaxTickets; i++) {
        mTickets[i] = nullptr;
    }
}

/** @brief Finishes the initialization of every registered camera. */
void CameraTicketHolder::endInit() {
    for (s32 i = 0; i < mNumTickets; i++) {
        mTickets[i]->getPoser()->endInit();
    }

    if (mDefaultTicket != nullptr) {
        mDefaultTicket->getPoser()->endInit();
    }
}

/**
 * @brief Registers a ticket, or makes it the default ticket if it has the default priority.
 * @param pTicket The ticket to register.
 */
void CameraTicketHolder::registerTicket(CameraTicket* pTicket) {
    if (pTicket->getPriority() == CameraTicket::Priority_Default) {
        registerDefaultTicket(pTicket);
        return;
    }

    mTickets[mNumTickets] = pTicket;
    mNumTickets++;
}

/**
 * @brief Sets the ticket of the camera used when no other camera is active.
 * @param pTicket The default ticket.
 */
void CameraTicketHolder::registerDefaultTicket(CameraTicket* pTicket) {
    mDefaultTicket = pTicket;
}

/**
 * @brief Looks for the entrance camera of a placement.
 * @param pPlacementId The placement id of the entrance.
 * @param pSuffix The suffix of the camera.
 * @return The ticket of the entrance camera, or null if there is none.
 */
CameraTicket* CameraTicketHolder::tryFindEntranceTicket(const PlacementId* pPlacementId, const char* pSuffix) const {
    CameraTicketId ticketId(pPlacementId, pSuffix);

    for (s32 i = 0; i < mNumTickets; i++) {
        if (mTickets[i]->getPriority() == CameraTicket::Priority_Entrance &&
            mTickets[i]->getTicketId()->isEqual(ticketId)) {
            return mTickets[i];
        }
    }

    return nullptr;
}
}  // namespace al
