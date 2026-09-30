#pragma once

#include <math/seadVector.h>

namespace al {
class CameraDirector;
class CameraDirector_RS;
class PlayerHolder;
struct PadRumbleParam;
class WaveVibrationHolder;

class PadRumbleDirector {
public:
    PadRumbleDirector(const PlayerHolder* pPlayerHolder, const CameraDirector_RS* pCameraDirector);
    PadRumbleDirector(const PlayerHolder* pPlayerHolder, const CameraDirector* pCameraDirector);

    void startRumble(const char* pName, const sead::Vector3f& rPos, const PadRumbleParam& rParam,
                     s32 port, bool isFlag);
    void startRumbleNo3D(const char* pName, const PadRumbleParam& rParam, s32 port, bool isFlag);
    void startRumbleLoop(const char* pName, const sead::Vector3f* pPos,
                         const PadRumbleParam& rParam, s32 port, bool isFlag);
    void startRumbleLoopNo3D(const char* pName, const sead::Vector3f* pPos,
                             const PadRumbleParam& rParam, s32 port, bool isFlag);
    void stopRumbleLoop(const char* pName, const sead::Vector3f* pPos, s32 port);
    bool checkIsAliveRumbleLoop(const char* pName, const sead::Vector3f* pPos, s32 port);
    void changeRumbleLoopVolume(const char* pName, const sead::Vector3f* pPos, f32 volumeLeft,
                                f32 volumeRight, s32 port);
    void changeRumbleLoopPitch(const char* pName, const sead::Vector3f* pPos, f32 pitchLeft,
                               f32 pitchRight, s32 port);
    void startRumbleDirectValue(f32 volumeLeft, f32 pitchLeft, f32 frequencyLeft, f32 volumeRight,
                                f32 pitchRight, f32 frequencyRight, s32 port);
    void stopRumbleDirectValue(s32 port);
    void startRumbleWithVolume(const char* pName, f32 volumeLeft, f32 volumeRight, s32 port);
    void stopPadRumbleOneTime(const char* pName, s32 port);
    void stopPadRumbleDirect(s32 port);
    void pauseActiveRumbles();
    void resumeActiveRumbles();
    void stopAllRumble();
    void update();
    void pause();
    void endPause();
    void setWaveVibrationHolder(WaveVibrationHolder* pHolder);

    void validate() { mIsValid = true; }
    void invalidate() { mIsValid = false; }
    void setPowerLevel(s32 level) { mPowerLevel = level; }

    u8 _0[0x41];
    bool mIsValid;
    u8 _42[6];
    s32 mPowerLevel;
    u8 _4c[0x90 - 0x4c];
};
}  // namespace al
