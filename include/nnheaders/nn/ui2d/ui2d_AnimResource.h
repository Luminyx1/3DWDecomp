#pragma once
#include <nn/types.h>
namespace nn::ui2d {
struct ResAnimationBlock;
struct ResAnimationGroup { char name[0x24]; };
class AnimResource {
public:
    void Set(const void* pResource);
    const char* GetTagName() const;
    u16 GetGroupCount() const;
    const ResAnimationGroup* GetGroupArray() const;
    const void* mFile;
    const ResAnimationBlock* mAnimation;
    const void* mTag;
    const void* mSharedAnimations;
};
static_assert(sizeof(AnimResource) == 0x20, "AnimResource size");
}
