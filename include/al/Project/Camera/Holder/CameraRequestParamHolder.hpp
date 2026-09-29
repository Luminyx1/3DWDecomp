#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseCamera;

/// A camera request flag together with the object that raised it.
struct CameraRequestFlag {
    bool mIsOn = false;                      // _0
    bool mIsDefaultOn = false;               // _1
    const IUseCamera* mRequester = nullptr;  // _8
};

/// Holds requests from objects that change how the player camera behaves.
class CameraRequestParamHolder {
public:
    CameraRequestParamHolder();

    void resetPlayerType();
    bool isPlayerTypeFlyer() const;
    void onPlayerTypeFlyer(const IUseCamera* pRequester, const char* pName);
    bool isPlayerTypeHighSpeedMove() const;
    void onPlayerTypeHighSpeedMove(const IUseCamera* pRequester, const char* pName);
    bool isPlayerTypeHighJump() const;
    void onPlayerTypeHighJump(const IUseCamera* pRequester, const char* pName);
    bool isPlayerTypeNotTouchGround() const;
    void onPlayerTypeNotTouchGround(const IUseCamera* pRequester, const char* pName);
    void onRideObj(const IUseCamera* pRequester, const char* pName);
    void offRideObj(const IUseCamera* pRequester, const char* pName);

    void* _0 = nullptr;                         // _0
    CameraRequestFlag mPlayerTypeFlyer;         // _8
    CameraRequestFlag mPlayerTypeHighSpeedMove;  // _18
    CameraRequestFlag mPlayerTypeHighJump;      // _28
    CameraRequestFlag mPlayerTypeNotTouchGround;  // _38
    CameraRequestFlag mRideObj;                 // _48
    void* _58 = nullptr;                        // _58
    void* _60 = nullptr;                        // _60
};
}  // namespace al
