#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ChikaChikaBlock : public al::LiveActor {
public:
    ChikaChikaBlock(const char*);
    int getSwitchTime() const;
    void onBgmSync();
    void startSwitchOffSign();
    void startSwitch();
    void startSwitchOn();
    void setBgmMute(bool mute) { mIsBgmMute = mute; }
    void disableSpeedMode() { mIsSpeedModeDisabled = true; }
private:
    unsigned char _144[0xc];
    bool mIsSpeedModeDisabled;
    unsigned char _151[7];
    bool mIsBgmMute;
    unsigned char _159[7];
};
