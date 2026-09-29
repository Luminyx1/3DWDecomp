#include <eui/euiPartsEx.h>

namespace eui {

/** @brief Creates an empty parts pane. */
PartsEx::PartsEx() = default;

/** @brief Builds a parts pane from layout resources. */
PartsEx::PartsEx(const nn::ui2d::ResParts* pResource,
                       const nn::ui2d::ResParts* pOverride,
                       const nn::ui2d::BuildArgSet& rArgs)
    : Parts(pResource, pOverride, rArgs) {}

/** @brief Copies a parts pane. */
PartsEx::PartsEx(const PartsEx& rOther) : Parts(rOther) {}

static_assert(sizeof(PartsEx) == 0xf0, "PartsEx size");

}  // namespace eui
