#include <eui/euiLayoutEx.h>
#include <eui/euiAnimator.h>
#include <eui/euiAnimatorSet.h>
#include <eui/euiMultiArcResourceAccessor.h>
#include <nn/ui2d/ui2d_AnimResource.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
namespace eui {
namespace {
// pLayout receives the animator; pResource supplies the animation data to bind.
inline Animator* CreateAnimator(LayoutEx* pLayout, const nn::ui2d::ResAnimationBlock* pResource) {
    if (pResource == nullptr) {
        return nullptr;
    }

    auto* device = reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice());
    void* memory = nn::ui2d::Layout::AllocateMemory(sizeof(Animator));

    if (memory == nullptr) {
        return nullptr;
    }

    auto* animator = new (memory) Animator;
    pLayout->mAnimTransformList.LinkPrev(&animator->m_Link);
    animator->SetResource(device, pLayout->mResourceAccessor, pResource);
    return animator;
}
}

// pScreen is the screen that owns this layout.
LayoutEx::LayoutEx(Screen* pScreen) : _60(nullptr), _68(nullptr), _70(nullptr), _78(nullptr),
    mScreen(pScreen), mParentLayout(nullptr), mFlags(0x200) {}
// pName identifies an animation; enabled sets its initial enabled state.
Animator* LayoutEx::createAnimatorAuto(const char* pName, bool enabled) {
    return tryCreateAnimatorAuto(pName, enabled);
}

// pName identifies the animation; enabled selects its initial enabled state.
Animator* LayoutEx::tryCreateAnimatorAutoWithWarning(const char* pName, bool enabled) {
    return tryCreateAnimatorAuto(pName, enabled);
}

// pName identifies a resource in this layout's archive, falling back to the base accessor.
const void* LayoutEx::GetAnimResourceData(const char* pName) {
    auto* accessor = DynamicCast<MultiArcResourceAccessor>(mResourceAccessor);

    if (accessor != nullptr) {
        return accessor->findAnimationResource(getLayoutName(), pName, nullptr);
    }

    return nn::ui2d::Layout::GetAnimResourceData(pName);
}

// pName identifies an animation with groups; enabled selects its initial enabled state.
Animator* LayoutEx::tryCreateAnimatorAuto(const char* pName, bool enabled) {
    const void* data = GetAnimResourceData(pName);

    if (data == nullptr) {
        return nullptr;
    }

    nn::ui2d::AnimResource resource;
    resource.Set(data);

    if (!resource.GetGroupCount()) {
        return nullptr;
    }

    Animator* animator = CreateAnimator(this, resource.mAnimation);
    animator->SetupWithGroupAll(resource, this, getGroupContainer(), enabled);
    return animator;
}

// pNames supplies count animation names; enabled enables only the first slot initially.
AnimatorSet* LayoutEx::createAnimatorSet(const char* const* pNames, u32 count, bool enabled) {
    void* memory = AllocateMemory(sizeof(AnimatorSet) + sizeof(Animator*) * size_t(count));

    if (memory == nullptr) {
        return nullptr;
    }

    auto* set = new (memory) AnimatorSet;
    set->setBuffer(count, reinterpret_cast<Animator**>(set + 1));

    for (size_t i = 0; i != count; ++i) {
        if (pNames[i] != nullptr && *pNames[i]) {
            set->setAnimator(i, tryCreateAnimatorAuto(pNames[i], enabled && i == 0));
        }
    }

    return set;
}

// pName supplies animation data for pPane; enabled sets the initial animation state.
Animator* LayoutEx::createAnimatorWithPane(const char* pName, nn::ui2d::Pane* pPane, bool enabled) {
    nn::ui2d::AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    Animator* animator = CreateAnimator(this, resource.mAnimation);
    animator->SetupWithPane(resource, this, pPane, enabled);
    return animator;
}

// pName supplies animation data for pGroup; enabled sets the initial animation state.
Animator* LayoutEx::createAnimatorWithGroup(const char* pName, nn::ui2d::Group* pGroup, bool enabled) {
    nn::ui2d::AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    Animator* animator = CreateAnimator(this, resource.mAnimation);
    animator->SetupWithGroup(resource, this, pGroup, enabled);
    return animator;
}

// pName identifies an animation, index selects its group, and enabled sets its initial state.
Animator* LayoutEx::createAnimatorWithGroupIndex(const char* pName, u32 index, bool enabled) {
    return tryCreateAnimatorWithGroupIndex(pName, index, enabled);
}

// pName identifies an animation, index selects a valid group, and enabled sets its initial state.
Animator* LayoutEx::tryCreateAnimatorWithGroupIndex(const char* pName, u32 index, bool enabled) {
    const void* data = GetAnimResourceData(pName);

    if (data == nullptr) {
        return nullptr;
    }

    nn::ui2d::AnimResource resource;
    resource.Set(data);

    if (index >= resource.GetGroupCount()) {
        return nullptr;
    }

    Animator* animator = CreateAnimator(this, resource.mAnimation);
    animator->SetupWithGroupIndex(resource, this, getGroupContainer(), index, enabled);
    return animator;
}

// pName identifies an animation left unbound to panes; enabled sets its initial state.
Animator* LayoutEx::createUnbindedAnimator(const char* pName, bool enabled) {
    nn::ui2d::AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    Animator* animator = CreateAnimator(this, resource.mAnimation);
    animator->SetupBasic(resource, this, enabled);
    return animator;
}

// pPane and rArgs identify the newly built pane and its build context; the base hook does nothing.
void LayoutEx::afterBuildPane_(nn::ui2d::Pane* pPane, const nn::ui2d::BuildArgSet& rArgs) {}
}
