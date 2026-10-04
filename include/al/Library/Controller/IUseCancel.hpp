#pragma once

namespace al {
class IUseCancel {
public:
    virtual void cancel() = 0;
};
}  // namespace al
