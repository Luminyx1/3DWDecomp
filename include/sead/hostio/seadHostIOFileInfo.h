#pragma once

#include "prim/seadSafeString.h"

namespace sead::hostio
{
class FileInfo
{
public:
    SafeString mPath;
    u32 _10;
};
}  // namespace sead::hostio
