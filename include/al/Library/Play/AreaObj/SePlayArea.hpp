#pragma once

#include "Project/AreaObj/AreaObj.hpp"

namespace al {
class SePlayArea : public AreaObj {
public:
    SePlayArea(const char*);

    void init(const AreaInitInfo& rInfo) override;

    const char* mPlayName = nullptr;
};
}  // namespace al
