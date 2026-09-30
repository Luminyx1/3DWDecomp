#pragma once

namespace al {
class BgmLine;

class IUseActiveBgmLine {
public:
    virtual BgmLine* getActiveBgmLine() const = 0;
};
}  // namespace al
