#pragma once

#include <basis/seadTypes.h>

#include "Library/Controller/GamePadSystem.hpp"

class GameDataHolder;

namespace al {
class ApplicationMessageReceiver;
class IUseCancel;
}  // namespace al

class ControllerConnectChecker : public al::GamePadSystemDelegate {
public:
    ControllerConnectChecker(GameDataHolder* pHolder,
                             const al::ApplicationMessageReceiver* pMessageReceiver,
                             al::GamePadSystem* pGamePadSystem);

    bool disconnect() override;
    void setCancelUser(al::IUseCancel* pCancelUser) override;
    void update();
    bool showControllerSupportApplet(u8& rPlayerCount) const;
    void disconnectControllers() const;
    void setAppletDisabled(bool isDisabled) override;
    void setDisabled(bool isDisabled) override;

private:
    GameDataHolder* mGameDataHolder;
    const al::ApplicationMessageReceiver* mMessageReceiver;
    al::GamePadSystem* mGamePadSystem;
    u32 mWaitFrame = 0;
    bool mIsConsoleMode;
    bool mIsAppletDisabled = false;
    bool mIsDisabled = false;
    al::IUseCancel* mCancelUser = nullptr;
};

static_assert(sizeof(ControllerConnectChecker) == 0x30);
