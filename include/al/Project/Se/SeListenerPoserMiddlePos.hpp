#pragma once

#include "Project/Se/SeListenerPoser.hpp"

namespace al {

class SeListenerPoserMiddlePos : public SeListenerPoser {
public:
    SeListenerPoserMiddlePos(const sead::SafeString& rName, const sead::SafeString& rGroupName,
                             f32 baseToMiddleRatio);
    void setBaseToMiddleRatio(f32 ratio);
    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;

private:
    f32 mBaseToMiddleRatio;
};

static_assert(sizeof(SeListenerPoserMiddlePos) == 0x20);

}  // namespace al
