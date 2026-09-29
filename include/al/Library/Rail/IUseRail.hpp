#pragma once

namespace al {
class RailKeeper;

class IUseRail {
public:
    virtual RailKeeper* getRailKeeper() const = 0;
};
}  // namespace al
