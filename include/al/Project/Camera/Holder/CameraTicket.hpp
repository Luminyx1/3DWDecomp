#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoser_RS;
class CameraTicketId;

/// A handle for starting and ending a camera poser with a priority.
class CameraTicket {
public:
    enum Priority {
        Priority_Default = 0,
        Priority_Entrance = 2,
    };

    CameraTicket(CameraPoser_RS* pPoser, const CameraTicketId* pTicketId, s32 priority);

    void setPriority(s32 priority);

    CameraPoser_RS* getPoser() const { return mPoser; }
    const CameraTicketId* getTicketId() const { return mTicketId; }
    s32 getPriority() const { return mPriority; }
    bool isActiveCamera() const { return mIsActiveCamera; }
    void setActiveCamera(bool isActiveCamera) { mIsActiveCamera = isActiveCamera; }

    CameraPoser_RS* mPoser;            // _0
    const CameraTicketId* mTicketId;   // _8
    s32 mPriority;                     // _10
    bool mIsActiveCamera = false;      // _14
    bool _15 = false;                  // _15
    bool _16 = false;                  // _16
};
}  // namespace al
