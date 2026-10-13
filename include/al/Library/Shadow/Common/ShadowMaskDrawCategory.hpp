#pragma once
#include <basis/seadTypes.h>
namespace al {
class ShadowMaskDrawCategory {
public:
  explicit ShadowMaskDrawCategory(s32 value) : mValue(value) {}

private:
  s32 mValue;
};
static_assert(sizeof(ShadowMaskDrawCategory) == 4);
} // namespace al
