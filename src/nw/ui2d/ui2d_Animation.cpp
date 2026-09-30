#include <nn/ui2d/ui2d_AnimTransform.h>
#include <nn/ui2d/ui2d_AnimResource.h>
#include <nn/ui2d/ui2d_Layout.h>

namespace nn::ui2d {
AnimTransform::AnimTransform() : m_pResource(nullptr), mFrame(0), mEnabled(true) {}
AnimTransform::~AnimTransform() = default;
u16 AnimTransform::GetFrameSize() const { return m_pResource->frameSize; }
// step is unused by the base transform; Animator supplies playback behavior.
void AnimTransform::UpdateFrame(float step) {}
// enabled controls whether the transform applies animation values.
void AnimTransform::SetEnabled(bool enabled) { mEnabled = enabled; }
bool AnimTransform::IsLoopData() const { return m_pResource->loop != 0; }
bool AnimTransform::IsWaitData() const { return m_pResource->frameSize == 0; }
AnimTransformBasic::AnimTransformBasic() : _28(nullptr), _30(nullptr), _38(0) {}
AnimTransformBasic::~AnimTransformBasic() {
    if (_30) Layout::FreeMemory(_30);
    if (_28) Layout::FreeMemory(_28);
}
// device and accessor resolve resources; resource supplies the default bind capacity.
void AnimTransformBasic::SetResource(nn::gfx::Device* device, ResourceAccessor* accessor, const ResAnimationBlock* resource) {
    SetResource(device, accessor, resource, resource->contentCount);
}
void AnimTransformBasic::ResetAnimResource() {
    m_pResource = nullptr;
    if (_30) Layout::FreeMemory(_30);
    _30 = nullptr;
    if (_28) Layout::FreeMemory(_28);
    _28 = nullptr;
}
void AnimTransformBasic::UnbindAll() { mBindCount = 0; }
void AnimResource::Initialize() { mFile = nullptr; mAnimation = nullptr; mTag = nullptr; mSharedAnimations = nullptr; }
bool AnimResource::CheckResource() const { return mAnimation != nullptr; }
u16 AnimResource::GetTagOrder() const { return mTag ? mTag->order : 0xffff; }
const char* AnimResource::GetTagName() const { return mTag ? reinterpret_cast<const char*>(mTag) + mTag->nameOffset : nullptr; }
u16 AnimResource::GetGroupCount() const { return mTag ? mTag->groupCount : 0; }
const ResAnimationGroup* AnimResource::GetGroupArray() const {
    return mTag ? reinterpret_cast<const ResAnimationGroup*>(reinterpret_cast<const char*>(mTag) + mTag->groupOffset) : nullptr;
}
const ResExtUserDataList* AnimResource::GetExtUserDataList() const {
    if (!mTag) return nullptr;
    if (!mTag->userDataOffset) return nullptr;
    return reinterpret_cast<const ResExtUserDataList*>(reinterpret_cast<const char*>(mTag) + mTag->userDataOffset);
}
bool AnimResource::IsDescendingBind() const { return mTag ? (mTag->flags & 1) != 0 : false; }
u16 AnimResource::GetAnimationShareInfoCount() const { return mSharedAnimations ? mSharedAnimations->infoCount : 0; }
const ResAnimationShareInfo* AnimResource::GetAnimationShareInfoArray() const {
    return mSharedAnimations ? reinterpret_cast<const ResAnimationShareInfo*>(reinterpret_cast<const char*>(mSharedAnimations) + mSharedAnimations->infoOffset) : nullptr;
}
}
