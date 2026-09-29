#pragma once

#include <nn/ui2d/ui2d_Bounding.h>

namespace eui {
class BoundingEx : public nn::ui2d::Bounding {
public:
    BoundingEx();
    BoundingEx(const nn::ui2d::ResBounding* pResource,
               const nn::ui2d::ResBounding* pOverride, const nn::ui2d::BuildArgSet& rArgs);
    BoundingEx(const BoundingEx& rOther);
    /** @brief Destroys the extended bounding pane. */
    ~BoundingEx() override = default;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Bounding);
};
}  // namespace eui
