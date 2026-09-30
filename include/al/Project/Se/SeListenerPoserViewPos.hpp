#pragma once

#include "Project/Se/SeListenerPoser.hpp"

namespace al {

class SeListenerPoserViewPos : public SeListenerPoser {
public:
    SeListenerPoserViewPos(const sead::SafeString& rName, const sead::SafeString& rGroupName);
    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;
};

}  // namespace al
