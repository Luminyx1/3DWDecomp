#pragma once

#include <cstdio>

namespace nn {
namespace util {
int SNPrintf(char*, size_t, const char*, ...);
int TSNPrintf(char*, size_t, const char*, ...);
};
};  // namespace nn