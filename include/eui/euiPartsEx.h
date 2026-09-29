#pragma once

#include <nn/ui2d/ui2d_Parts.h>

namespace eui {
class PartsEx : public nn::ui2d::Parts {
public:
    PartsEx();
    PartsEx(const nn::ui2d::ResParts* pResource,
               const nn::ui2d::ResParts* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    PartsEx(const PartsEx& rOther);
    /** @brief Destroys the extended bounding pane. */
    ~PartsEx() override = default;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Parts);
};
}  // namespace eui
