#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace agl::utl {
class ParameterObj;
}

namespace al {

class DepthOfFieldParam {
public:
    DepthOfFieldParam();

    agl::utl::ParameterObj* getParamObj() const { return mParamObj; }

private:
    agl::utl::ParameterObj* mParamObj;
    u8 _8[0x50];
};

static_assert(sizeof(DepthOfFieldParam) == 0x58);

class DepthOfFieldDrawer : public NerveExecutor {
public:
    ~DepthOfFieldDrawer() override;
};

}  // namespace al
