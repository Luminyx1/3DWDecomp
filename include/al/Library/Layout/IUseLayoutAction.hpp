#pragma once

#include "Library/HostIO/IUseName.hpp"

namespace al {
class LayoutActionKeeper;

class IUseLayoutAction : virtual public IUseName {
public:
    virtual LayoutActionKeeper* getLayoutActionKeeper() const = 0;
};
}  // namespace al
