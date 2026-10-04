#pragma once

#include "Project/AreaObj/AreaObj.hpp"

class NoRainArea : public al::AreaObj {
public:
    NoRainArea(const char* pName);

    void init(const al::AreaInitInfo& rInfo) override;

    bool mIsIgnoreCamera = false;  // 0x84
};
