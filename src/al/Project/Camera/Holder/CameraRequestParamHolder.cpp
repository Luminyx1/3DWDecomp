#include "Project/Camera/Holder/CameraRequestParamHolder.hpp"

namespace al {

CameraRequestParamHolder::CameraRequestParamHolder() = default;

void CameraRequestParamHolder::resetPlayerType() {
    mIsCurrFlyer = mIsPrevFlyer;
    mFlyerCamera = nullptr;
    mIsCurrHighSpeedMove = mIsPrevHighSpeedMove;
    mHighSpeedMoveCamera = nullptr;
    mIsCurrHighJump = mIsPrevHighJump;
    mHighJumpCamera = nullptr;
    mIsCurrNotTouchGround = mIsPrevNotTouchGround;
    mNotTouchGroundCamera = nullptr;
}

bool CameraRequestParamHolder::isPlayerTypeFlyer() const {
    return (mFlyerCamera != nullptr) && mIsCurrFlyer;
}

void CameraRequestParamHolder::onPlayerTypeFlyer(const IUseCamera* pCamera, const char* pName) {
    mIsCurrFlyer = true;
    mFlyerCamera = pCamera;
}

bool CameraRequestParamHolder::isPlayerTypeHighSpeedMove() const {
    return (mHighSpeedMoveCamera != nullptr) && mIsCurrHighSpeedMove;
}

void CameraRequestParamHolder::onPlayerTypeHighSpeedMove(const IUseCamera* pCamera,
                                                         const char* pName) {
    mIsCurrHighSpeedMove = true;
    mHighSpeedMoveCamera = pCamera;
}

bool CameraRequestParamHolder::isPlayerTypeHighJump() const {
    return (mHighJumpCamera != nullptr) && mIsCurrHighJump;
}

void CameraRequestParamHolder::onPlayerTypeHighJump(const IUseCamera* pCamera, const char* pName) {
    mIsCurrHighJump = true;
    mHighJumpCamera = pCamera;
}

bool CameraRequestParamHolder::isPlayerTypeNotTouchGround() const {
    return (mNotTouchGroundCamera != nullptr) && mIsCurrNotTouchGround;
}

void CameraRequestParamHolder::onPlayerTypeNotTouchGround(const IUseCamera* pCamera,
                                                          const char* pName) {
    mIsCurrNotTouchGround = true;
    mNotTouchGroundCamera = pCamera;
}

void CameraRequestParamHolder::onRideObj(const IUseCamera* pCamera, const char* pName) {
    mIsCurrRideObj = true;
    mRideObjCamera = pCamera;
}

void CameraRequestParamHolder::offRideObj(const IUseCamera* pCamera, const char* pName) {
    mRideObjCamera = nullptr;
    mIsCurrRideObj = mIsPrevRideObj;
}

}  // namespace al
