#include "audio/seadAudio3DListenerNin.h"

#include <nn/util/util_MatrixApi.h>
#include <nn/util/util_VectorApi.h>

namespace sead {
/**
 * Constructs a listener that follows every parameter of its group.
 */
Audio3DListenerNin::Audio3DListenerNin() = default;

/** @brief Destroys the listener without owning its group or audio resources. */
Audio3DListenerNin::~Audio3DListenerNin() = default;

/**
 * Sets the listener matrix.
 * @param rMtx Listener matrix.
 */
void Audio3DListenerNin::setMatrix(const Matrix34f& rMtx) {
    nn::util::Matrix4x3fType mtx;
    nn::util::MatrixLoad(&mtx, reinterpret_cast<const nn::util::FloatColumnMajor4x3&>(rMtx));
    SetMatrix(mtx);
}

/**
 * Gets the listener matrix.
 * @return Listener matrix.
 */
Matrix34f Audio3DListenerNin::getMatrix() const {
    Matrix34f mtx;
    nn::util::MatrixStore(reinterpret_cast<nn::util::FloatColumnMajor4x3*>(&mtx), GetMatrix());
    return mtx;
}

/**
 * Resets the listener matrix so that the next matrix is not used for velocity.
 */
void Audio3DListenerNin::resetMatrix() {
    ResetMatrix();
}

/**
 * Gets the listener position.
 * @return Position.
 */
Vector3f Audio3DListenerNin::getPosition() const {
    const nn::util::Vector3fType& position = GetPosition();
    return Vector3f(nn::util::VectorGetX(position), nn::util::VectorGetY(position),
                    nn::util::VectorGetZ(position));
}

/**
 * Sets the listener velocity.
 * @param rVelocity Velocity.
 */
void Audio3DListenerNin::setVelocity(const Vector3f& rVelocity) {
    nn::util::Vector3fType velocity;
    velocity._v[0] = rVelocity.x;
    velocity._v[1] = rVelocity.y;
    velocity._v[2] = rVelocity.z;
    SetVelocity(velocity);
}

/**
 * Gets the listener velocity.
 * @return Velocity.
 */
Vector3f Audio3DListenerNin::getVelocity() const {
    const nn::util::Vector3fType& velocity = GetVelocity();
    return Vector3f(nn::util::VectorGetX(velocity), nn::util::VectorGetY(velocity),
                    nn::util::VectorGetZ(velocity));
}

/**
 * Sets the interior size.
 * @param size Interior size.
 */
void Audio3DListenerNin::setInteriorSize(f32 size) {
    SetInteriorSize(size);
}

/**
 * Sets the maximum volume distance.
 * @param distance Maximum volume distance.
 */
void Audio3DListenerNin::setMaxVolumeDistance(f32 distance) {
    SetMaxVolumeDistance(distance);
}

/**
 * Sets the unit distance.
 * @param distance Unit distance.
 */
void Audio3DListenerNin::setUnitDistance(f32 distance) {
    SetUnitDistance(distance);
}

/**
 * Sets the biquad filter value per unit distance.
 * @param value Filter value.
 */
void Audio3DListenerNin::setUnitBiquadFilterValue(f32 value) {
    SetUnitBiquadFilterValue(value);
}

/**
 * Sets the maximum biquad filter value.
 * @param value Filter value.
 */
void Audio3DListenerNin::setMaxBiquadFilterValue(f32 value) {
    SetMaxBiquadFilterValue(value);
}

/**
 * Sets the user parameter.
 * @param param User parameter.
 */
void Audio3DListenerNin::setUserParam(u32 param) {
    SetUserParam(param);
}

/**
 * Sets the output type flags.
 * @param flag Output type flags.
 */
void Audio3DListenerNin::setOutputTypeFlag(u32 flag) {
    SetOutputTypeFlag(flag);
}

/**
 * Sets the output type (unsupported, does nothing).
 * @param type Output type.
 */
void Audio3DListenerNin::setOutputType(nn::atk::Sound3DListener::ListenerOutputType type) {}

/**
 * Sets every listener parameter.
 * @param rParam Listener parameters.
 */
void Audio3DListenerNin::setParameterAll(const Audio3DListenerParameterNin& rParam) {
    SetInteriorSize(rParam.mInteriorSize);
    SetMaxVolumeDistance(rParam.mMaxVolumeDistance);
    SetUnitDistance(rParam.mUnitDistance);
    SetUnitBiquadFilterValue(rParam.mUnitBiquadFilterValue);
    SetMaxBiquadFilterValue(rParam.mMaxBiquadFilterValue);
    SetUserParam(rParam.mUserParam);
    SetOutputTypeFlag(rParam.mOutputTypeFlag);
}

/**
 * Gets the interior size.
 * @return Interior size.
 */
f32 Audio3DListenerNin::getInteriorSize() const {
    return GetInteriorSize();
}

/**
 * Gets the maximum volume distance.
 * @return Maximum volume distance.
 */
f32 Audio3DListenerNin::getMaxVolumeDistance() const {
    return GetMaxVolumeDistance();
}

/**
 * Gets the unit distance.
 * @return Unit distance.
 */
f32 Audio3DListenerNin::getUnitDistance() const {
    return GetUnitDistance();
}

/**
 * Gets the biquad filter value per unit distance.
 * @return Filter value.
 */
f32 Audio3DListenerNin::getUnitBiquadFilterValue() const {
    return GetUnitBiquadFilterValue();
}

/**
 * Gets the maximum biquad filter value.
 * @return Filter value.
 */
f32 Audio3DListenerNin::getMaxBiquadFilterValue() const {
    return GetMaxBiquadFilterValue();
}

/**
 * Gets the user parameter.
 * @return User parameter.
 */
u32 Audio3DListenerNin::getUserParam() const {
    return GetUserParam();
}

/**
 * Gets the output type flags.
 * @return Output type flags.
 */
u32 Audio3DListenerNin::getOutputTypeFlag() const {
    return GetOutputTypeFlag();
}

/**
 * Gets every listener parameter.
 * @param pParam Receives the listener parameters.
 */
void Audio3DListenerNin::getParameterAll(Audio3DListenerParameterNin* pParam) const {
    pParam->mInteriorSize = GetInteriorSize();
    pParam->mMaxVolumeDistance = GetMaxVolumeDistance();
    pParam->mUnitDistance = GetUnitDistance();
    pParam->mUserParam = GetUserParam();
    pParam->mUnitBiquadFilterValue = GetUnitBiquadFilterValue();
    pParam->mMaxBiquadFilterValue = GetMaxBiquadFilterValue();
    pParam->mOutputTypeFlag = GetOutputTypeFlag();
}

/**
 * Generates the host IO message (stripped in release builds).
 * @param pContext Host IO context.
 */
void Audio3DListenerNin::genMessage(hostio::Context* pContext) {}

/**
 * Handles a host IO property event (no-op in release builds).
 * @param pEvent Property event.
 */
void Audio3DListenerNin::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {}
}  // namespace sead
