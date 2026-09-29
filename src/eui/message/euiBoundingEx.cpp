#include <eui/euiBoundingEx.h>

namespace eui {

/** @brief Creates an empty bounding pane. */
BoundingEx::BoundingEx() = default;

/**
 * @brief Builds a bounding pane from layout resources.
 * @param[in] pResource Base pane resource to construct from.
 * @param[in] pOverride Override pane resource passed to the NintendoWare constructor.
 * @param[in] rArgs Layout construction arguments and resource context.
 */
BoundingEx::BoundingEx(const nn::ui2d::ResBounding* pResource,
                       const nn::ui2d::ResBounding* pOverride,
                       const nn::ui2d::BuildArgSet& rArgs)
    : Bounding(pResource, pOverride, rArgs) {}

/**
 * @brief Copies a bounding pane.
 * @param[in] rOther Source object to copy.
 */
BoundingEx::BoundingEx(const BoundingEx& rOther) : Bounding(rOther) {}

static_assert(sizeof(BoundingEx) == 0xd8, "BoundingEx size");

}  // namespace eui
