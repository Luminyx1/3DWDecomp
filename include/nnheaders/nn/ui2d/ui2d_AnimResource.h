#pragma once
#include <nn/types.h>
namespace nn::ui2d {
struct ResAnimationBlock {
    u32 signature, size;
    u16 frameSize;
    u8 loop, padding;
    u16 textureCount, contentCount;
    u32 contentOffsets;
};
struct ResAnimationTagBlock {
    u32 signature, size;
    u16 order, groupCount;
    u32 nameOffset, groupOffset, userDataOffset;
    u32 _18;
    u8 flags;
};
struct ResAnimationShareBlock {
    u32 signature, size, infoOffset;
    u16 infoCount;
};
struct ResExtUserDataList;
struct ResAnimationShareInfo;
struct ResAnimationGroup { char name[0x24]; };
class AnimResource {
public:
    void Initialize();
    bool CheckResource() const;
    u16 GetTagOrder() const;
    const ResExtUserDataList* GetExtUserDataList() const;
    bool IsDescendingBind() const;
    u16 GetAnimationShareInfoCount() const;
    const ResAnimationShareInfo* GetAnimationShareInfoArray() const;
    void Set(const void* pResource);
    const char* GetTagName() const;
    u16 GetGroupCount() const;
    const ResAnimationGroup* GetGroupArray() const;
    const void* mFile;
    const ResAnimationBlock* mAnimation;
    const ResAnimationTagBlock* mTag;
    const ResAnimationShareBlock* mSharedAnimations;
};
static_assert(sizeof(AnimResource) == 0x20, "AnimResource size");
}
