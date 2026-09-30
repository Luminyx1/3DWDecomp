#pragma once

#include "Project/Se/SeListenerPoserMiddlePos.hpp"

namespace al {

class SeListenerPoserAdjustMiddlePos : public SeListenerPoserMiddlePos {
public:
    SeListenerPoserAdjustMiddlePos(const sead::SafeString& rName, const sead::SafeString& rGroupName);
    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;

private:
    f32 mPrevFovyDegree = 0.0f;
};

static_assert(sizeof(SeListenerPoserAdjustMiddlePos) == 0x20);

}  // namespace al
