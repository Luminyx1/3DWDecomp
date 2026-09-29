#pragma once

#include <basis/seadTypes.h>

namespace sead::hostio
{
// TODO: layout
class NodeEvent
{
public:
    u32 getId() const { return mId; }

private:
    u32 mType;
    u32 _4;
    u32 mId;
};

}  // namespace sead::hostio
