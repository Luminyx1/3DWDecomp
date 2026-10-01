#pragma once

#include "Project/AreaObj/AreaObj.hpp"

namespace al {
class SwitchTriggerArea : public AreaObj {
public:
    SwitchTriggerArea(const char* pName);

    void init(const AreaInitInfo& rInfo) override;
    bool isInVolume(const sead::Vector3f& rPos) const override;

    using AreaObj::init;
    using AreaObj::isInVolume;
};
}  // namespace al
