#pragma once

#include <nn/util/util_IntrusiveList.h>
#include <nn/util/util_MathTypes.h>

namespace nn::atk {
class Sound3DListener {
public:
    enum ListenerOutputType {
        ListenerOutputType_Tv = (1 << 0),
        ListenerOutputType_Drc = (1 << 1)
    };

    Sound3DListener();

    void SetMatrix(const util::Matrix4x3fType& rMtx);
    const util::Matrix4x3fType& GetMatrix() const { return m_Matrix; }
    void ResetMatrix();
    const util::Vector3fType& GetPosition() const { return m_Position; }
    void SetVelocity(const util::Vector3fType& rVelocity);
    const util::Vector3fType& GetVelocity() const { return m_Velocity; }
    void SetInteriorSize(f32 interiorSize);
    f32 GetInteriorSize() const { return m_InteriorSize; }
    void SetMaxVolumeDistance(f32 maxVolumeDistance);
    f32 GetMaxVolumeDistance() const { return m_MaxVolumeDistance; }
    void SetUnitDistance(f32 unitDistance);
    f32 GetUnitDistance() const { return m_UnitDistance; }
    void SetUserParam(u32 param) { m_UserParam = param; }
    u32 GetUserParam() const { return m_UserParam; }
    void SetUnitBiquadFilterValue(f32 value);
    f32 GetUnitBiquadFilterValue() const { return m_UnitBiquadFilterValue; }
    void SetMaxBiquadFilterValue(f32 value);
    f32 GetMaxBiquadFilterValue() const { return m_MaxBiquadFilterValue; }
    void SetOutputTypeFlag(u32 outputTypeFlag) { m_OutputTypeFlag = outputTypeFlag; }
    u32 GetOutputTypeFlag() const { return m_OutputTypeFlag; }

private:
    util::Matrix4x3fType m_Matrix;
    util::Vector3fType m_Position;
    util::Vector3fType m_Velocity;
    f32 m_InteriorSize;
    f32 m_MaxVolumeDistance;
    f32 m_UnitDistance;
    u32 m_UserParam;
    f32 m_UnitBiquadFilterValue;
    f32 m_MaxBiquadFilterValue;
    u32 m_OutputTypeFlag;
    bool m_ResetMatrixFlag;
    bool m_SkipVelocityUpdate;
    bool m_IsVelocityValid;

public:
    util::IntrusiveListNode m_LinkNode;
};
static_assert(sizeof(Sound3DListener) == 0x90);
}  // namespace nn::atk
