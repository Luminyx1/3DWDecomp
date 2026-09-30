#include <nn/atk/atk_Sound3DListener.h>
#include <cstring>

namespace nn::atk {
// matrix supplies the listener transform; successive positions determine velocity.
void Sound3DListener::SetMatrix(const nn::util::Matrix4x3fType& matrix) {
    auto translation = matrix._m.val[3];
    float x = -(vgetq_lane_f32(matrix._m.val[0], 0) * vgetq_lane_f32(translation, 0)
              + vgetq_lane_f32(matrix._m.val[0], 1) * vgetq_lane_f32(translation, 1)
              + vgetq_lane_f32(matrix._m.val[0], 2) * vgetq_lane_f32(translation, 2));
    float y = -(vgetq_lane_f32(matrix._m.val[1], 0) * vgetq_lane_f32(translation, 0)
              + vgetq_lane_f32(matrix._m.val[1], 1) * vgetq_lane_f32(translation, 1)
              + vgetq_lane_f32(matrix._m.val[1], 2) * vgetq_lane_f32(translation, 2));
    float z = -(vgetq_lane_f32(matrix._m.val[2], 0) * vgetq_lane_f32(translation, 0)
              + vgetq_lane_f32(matrix._m.val[2], 1) * vgetq_lane_f32(translation, 1)
              + vgetq_lane_f32(matrix._m.val[2], 2) * vgetq_lane_f32(translation, 2));
    auto previous = m_Position._v;
    float32x4_t position = vdupq_n_f32(0);
    position = vsetq_lane_f32(x, position, 0);
    position = vsetq_lane_f32(y, position, 1);
    position = vsetq_lane_f32(z, position, 2);
    m_Position._v = position;
    m_Matrix = matrix;

    if (m_ResetMatrixFlag) m_ResetMatrixFlag = false;
    else m_Velocity._v = vsubq_f32(position, previous);
}

void Sound3DListener::ResetMatrix() {
    std::memset(&m_Matrix, 0, sizeof(m_Matrix));
    m_ResetMatrixFlag = true;
    m_Velocity._v = vdupq_n_f32(0);
}

// velocity replaces the movement inferred from consecutive listener matrices.
void Sound3DListener::SetVelocity(const nn::util::Vector3fType& velocity) { m_Velocity = velocity; }
// size is the listener's interior region radius.
void Sound3DListener::SetInteriorSize(float size) { m_InteriorSize = size; }
// distance is the radius within which sounds retain maximum volume.
void Sound3DListener::SetMaxVolumeDistance(float distance) { m_MaxVolumeDistance = distance; }
// distance defines the listener's unit of distance for attenuation.
void Sound3DListener::SetUnitDistance(float distance) { m_UnitDistance = distance; }
// value is the biquad-filter attenuation per distance unit.
void Sound3DListener::SetUnitBiquadFilterValue(float value) { m_UnitBiquadFilterValue = value; }
// value limits the biquad-filter attenuation.
void Sound3DListener::SetMaxBiquadFilterValue(float value) { m_MaxBiquadFilterValue = value; }
}
