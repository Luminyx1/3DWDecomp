#pragma once

#include "heap/seadDisposer.h"
#include "random/seadRandom.h"

namespace sead
{
class GlobalRandom : public Random
{
    SEAD_SINGLETON_DISPOSER(GlobalRandom)
    GlobalRandom() = default;
};

/// A global random number generator for use from a single thread.
class GlobalRandomNonSync : public Random
{
    SEAD_SINGLETON_DISPOSER(GlobalRandomNonSync)
    GlobalRandomNonSync() = default;
};
}  // namespace sead
