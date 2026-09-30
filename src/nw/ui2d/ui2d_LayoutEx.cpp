#include <nn/ui2d/ui2d_LayoutEx.h>
#include <nn/ui2d/ui2d_AnimatorEx.h>
namespace nn::ui2d {
// screen owns the layout and its active animators.
LayoutEx::LayoutEx(Screen* screen) : mScreen(screen), mInOutAnimator(nullptr), mLoopAnimator(nullptr), mDefaultAnimator(nullptr), mAnimationState(2) {}
// name selects an AnimatorEx by its animation tag, compared through 64 characters.
AnimatorEx* LayoutEx::FindAnimator(const char* name) {
    for (auto* node = mAnimTransformList.GetNext(); node != &mAnimTransformList; node = node->GetNext()) {
        auto* transform = reinterpret_cast<AnimTransform*>(reinterpret_cast<char*>(node) - 8);
        const auto* wanted = AnimatorEx::GetRuntimeTypeInfoStatic();
        const auto* type = transform->GetRuntimeTypeInfo();
        while (type && type != wanted) type = type->m_ParentTypeInfo;
        auto* animator = type ? static_cast<AnimatorEx*>(transform) : nullptr;
        if (!animator || !animator->GetTagName()) continue;
        const char* tag = animator->GetTagName();
        bool same = true;
        for (size_t i = 0; i < 64; ++i) {
            if (tag[i] != name[i]) { same = false; break; }
            if (!tag[i]) break;
        }
        if (same) return animator;
    }
    return nullptr;
}
}
