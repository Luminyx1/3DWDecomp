#include "Project/Base/RequestInterp.hpp"

/**
 * @brief Returns the larger of two integers.
 * @param a The first value.
 * @param b The second value.
 * @return The larger of the two values.
 */
s32 RequestInterpMathImpl::max(s32 a, s32 b) {
    return a > b ? a : b;
}
