#include <eui/euiPartsEx.h>

namespace eui {

/** @brief Creates an empty parts pane. */
PartsEx::PartsEx() = default;

/**
 * @brief Builds a parts pane from layout resources.
 * @param[in] pResource Base pane resource to construct from.
 * @param[in] pOverride Override pane resource passed to the NintendoWare constructor.
 * @param[in] rArgs Layout construction arguments and resource context.
 */
PartsEx::PartsEx(const nn::ui2d::ResParts* pResource,
                       const nn::ui2d::ResParts* pOverride,
                       const nn::ui2d::BuildArgSet& rArgs)
    : Parts(pResource, pOverride, rArgs) {}

/**
 * @brief Copies a parts pane.
 * @param[in] rOther Source object to copy.
 */
PartsEx::PartsEx(const PartsEx& rOther) : Parts(rOther) {}

static_assert(sizeof(PartsEx) == 0xf0, "PartsEx size");

}  // namespace eui
