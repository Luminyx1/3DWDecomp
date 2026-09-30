#include "Project/Base/RequestInterp.hpp"

/**
 * Gets the larger of two values.
 * @param a First value.
 * @param b Second value.
 * @return The maximum.
 */
s32 RequestInterpMathImpl::max(s32 a, s32 b) {
    return a > b ? a : b;
}
