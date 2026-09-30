#include "Library/Camera/SpecialCameraHolder.hpp"

#include "Library/Camera/CameraTicket.hpp"
#include "Library/Camera/CameraTicketId.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

SpecialCameraHolder::SpecialCameraHolder() = default;

void SpecialCameraHolder::allocEntranceCameraBuffer(s32 maxEntries) {
    mMaxEntranceCameras = maxEntries;
    mEntranceCameras = new CameraTicket*[maxEntries];
    for (s32 i = 0; i < mMaxEntranceCameras; i++) {
        mEntranceCameras[i] = nullptr;
    }
}

void SpecialCameraHolder::registerEntranceCamera(CameraTicket* pTicket) {
    mEntranceCameras[mEntranceCameraNum] = pTicket;
    mEntranceCameraNum++;
}

CameraTicket* SpecialCameraHolder::findEntranceCamera(const char* pSuffix) const {
    for (s32 i = 0; i < mEntranceCameraNum; i++) {
        CameraTicket* ticket = mEntranceCameras[i];
        if (isEqualString(ticket->getTicketId()->getSuffix(), pSuffix)) {
            return ticket;
        }
    }
    return nullptr;
}

}  // namespace al
