#include "Project/Camera/Holder/CameraRequestParamHolder.hpp"

namespace al {
/** @brief Creates the holder without any requests. */
CameraRequestParamHolder::CameraRequestParamHolder() = default;

/** @brief Clears the player type requests, going back to their default state. */
void CameraRequestParamHolder::resetPlayerType() {
    mPlayerTypeFlyer.mIsOn = mPlayerTypeFlyer.mIsDefaultOn;
    mPlayerTypeFlyer.mRequester = nullptr;
    mPlayerTypeHighSpeedMove.mIsOn = mPlayerTypeHighSpeedMove.mIsDefaultOn;
    mPlayerTypeHighSpeedMove.mRequester = nullptr;
    mPlayerTypeHighJump.mIsOn = mPlayerTypeHighJump.mIsDefaultOn;
    mPlayerTypeHighJump.mRequester = nullptr;
    mPlayerTypeNotTouchGround.mIsOn = mPlayerTypeNotTouchGround.mIsDefaultOn;
    mPlayerTypeNotTouchGround.mRequester = nullptr;
}

/**
 * @brief Checks whether the player is treated as a flyer.
 * @return True if an object requested the flyer camera.
 */
bool CameraRequestParamHolder::isPlayerTypeFlyer() const {
    return mPlayerTypeFlyer.mRequester != nullptr && mPlayerTypeFlyer.mIsOn;
}

/**
 * @brief Requests the flyer camera.
 * @param pRequester The object requesting it.
 * @param pName The name of the request (unused).
 */
void CameraRequestParamHolder::onPlayerTypeFlyer(const IUseCamera* pRequester, const char* pName) {
    mPlayerTypeFlyer.mIsOn = true;
    mPlayerTypeFlyer.mRequester = pRequester;
}

/**
 * @brief Checks whether the player is treated as moving at high speed.
 * @return True if an object requested the high speed camera.
 */
bool CameraRequestParamHolder::isPlayerTypeHighSpeedMove() const {
    return mPlayerTypeHighSpeedMove.mRequester != nullptr && mPlayerTypeHighSpeedMove.mIsOn;
}

/**
 * @brief Requests the high speed camera.
 * @param pRequester The object requesting it.
 * @param pName The name of the request (unused).
 */
void CameraRequestParamHolder::onPlayerTypeHighSpeedMove(const IUseCamera* pRequester, const char* pName) {
    mPlayerTypeHighSpeedMove.mIsOn = true;
    mPlayerTypeHighSpeedMove.mRequester = pRequester;
}

/**
 * @brief Checks whether the player is treated as jumping high.
 * @return True if an object requested the high jump camera.
 */
bool CameraRequestParamHolder::isPlayerTypeHighJump() const {
    return mPlayerTypeHighJump.mRequester != nullptr && mPlayerTypeHighJump.mIsOn;
}

/**
 * @brief Requests the high jump camera.
 * @param pRequester The object requesting it.
 * @param pName The name of the request (unused).
 */
void CameraRequestParamHolder::onPlayerTypeHighJump(const IUseCamera* pRequester, const char* pName) {
    mPlayerTypeHighJump.mIsOn = true;
    mPlayerTypeHighJump.mRequester = pRequester;
}

/**
 * @brief Checks whether the player is treated as not touching the ground.
 * @return True if an object requested the camera for a player off the ground.
 */
bool CameraRequestParamHolder::isPlayerTypeNotTouchGround() const {
    return mPlayerTypeNotTouchGround.mRequester != nullptr && mPlayerTypeNotTouchGround.mIsOn;
}

/**
 * @brief Requests the camera for a player off the ground.
 * @param pRequester The object requesting it.
 * @param pName The name of the request (unused).
 */
void CameraRequestParamHolder::onPlayerTypeNotTouchGround(const IUseCamera* pRequester, const char* pName) {
    mPlayerTypeNotTouchGround.mIsOn = true;
    mPlayerTypeNotTouchGround.mRequester = pRequester;
}

/**
 * @brief Notifies that the player rides an object.
 * @param pRequester The ridden object.
 * @param pName The name of the request (unused).
 */
void CameraRequestParamHolder::onRideObj(const IUseCamera* pRequester, const char* pName) {
    mRideObj.mIsOn = true;
    mRideObj.mRequester = pRequester;
}

/**
 * @brief Notifies that the player stopped riding an object.
 * @param pRequester The object that was ridden (unused).
 * @param pName The name of the request (unused).
 */
void CameraRequestParamHolder::offRideObj(const IUseCamera* pRequester, const char* pName) {
    mRideObj.mRequester = nullptr;
    mRideObj.mIsOn = mRideObj.mIsDefaultOn;
}
}  // namespace al
