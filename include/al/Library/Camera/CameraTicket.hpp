#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoser_RS;
class CameraTicketId;

class CameraTicket {
public:
    enum Priority {
        Priority_Default = 0,
        Priority_Area = 1,
        Priority_Entrance = 2,
        Priority_BossField = 3,
        Priority_Capture = 4,
        Priority_Dokan = 5,
        Priority_Object = 6,
        Priority_ForceArea = 8,
        Priority_EntranceSub = 9,
        Priority_SafetyPointRecovery = 10,
        Priority_Player = 11,
        Priority_DemoTalk = 12,
        Priority_Demo = 13,
        Priority_Demo2 = 14,
    };

    CameraTicket(CameraPoser_RS* pPoser, const CameraTicketId* pTicketId, s32 priority);
    void setPriority(s32 priority);

    CameraPoser_RS* getPoser() const { return mPoser; }

    /**
     * @brief Get the camera poser as its concrete type.
     * @return The camera poser.
     */
    template <typename T>
    T* getPoser() const {
        return static_cast<T*>(mPoser);
    }

    const CameraTicketId* getTicketId() const { return mTicketId; }

    s32 getPriority() const { return mPriority; }

    bool isActiveCamera() const { return mIsActiveCamera; }

    void setActiveCamera(bool isActive) { mIsActiveCamera = isActive; }

    bool is15() const { return _15; }

    void setDisaster() { _15 = true; }

private:
    CameraPoser_RS* mPoser;
    const CameraTicketId* mTicketId;
    s32 mPriority;
    bool mIsActiveCamera = false;
    bool _15 = false;
    bool _16 = false;
};

static_assert(sizeof(CameraTicket) == 0x18);

}  // namespace al
