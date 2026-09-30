#pragma once

#include "Project/Se/SeListenerPoserViewPosOffset.hpp"

namespace al {

class SeListenerPoserViewPosOffsetFovy : public SeListenerPoserViewPosOffset {
public:
    SeListenerPoserViewPosOffsetFovy(const sead::SafeString& rName, const sead::SafeString& rGroupName);
    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;
};

}  // namespace al
