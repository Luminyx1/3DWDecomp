#pragma once

#include <prim/seadSafeString.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"

namespace al {
class GraphicsNamedParamBase : public agl::utl::IParameterObj {
public:
    GraphicsNamedParamBase(const char* pDefaultName);

    virtual s32 getParamType() const = 0;

    void interp(const GraphicsNamedParamBase& rA, const GraphicsNamedParamBase& rB, f32 rate);
    bool operator==(const GraphicsNamedParamBase& rOther) const;

    const char* getName() const { return (*mName)->cstr(); }

private:
    agl::utl::Parameter<sead::FixedSafeString<64>>* mName = nullptr;
};

static_assert(sizeof(GraphicsNamedParamBase) == 0x38);

}  // namespace al
