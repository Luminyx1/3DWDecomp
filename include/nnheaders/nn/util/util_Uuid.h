#pragma once

#include <nn/types.h>

namespace nn {
namespace util {

/**
 * @brief 128-bit universally unique identifier.
 */
struct Uuid {
    u8 data[16];
};

Uuid GenerateUuid();

}  // namespace util
}  // namespace nn
