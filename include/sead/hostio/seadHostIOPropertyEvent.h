#pragma once

#include <basis/seadTypes.h>

namespace sead::hostio
{
class PropertyEvent
{
public:
    const void* getId() const { return mId; }
    u32 getType() const { return mType; }

private:
    u32 mType;
    const void* mId;
};

}  // namespace sead::hostio
