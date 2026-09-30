#pragma once

#include "Project/Se/SeListenerPoser.hpp"

namespace al {

class SeListenerPoserViewPosOffset : public SeListenerPoser {
public:
    SeListenerPoserViewPosOffset(const sead::SafeString& rName, const sead::SafeString& rGroupName,
                                 const sead::Vector3f& rOffset);
    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;

protected:
    sead::Vector3f mOffset;
};

static_assert(sizeof(SeListenerPoserViewPosOffset) == 0x28);

}  // namespace al
