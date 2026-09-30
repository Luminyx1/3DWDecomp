#include "Library/Layout/LayoutKeeper.hpp"

#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <gfx/nin/seadGraphicsNvn.h>

#include "Library/Layout/LayoutResource.hpp"

namespace al {
namespace {
template <typename T>
T* dynamicCastResourceAccessor(nn::ui2d::ResourceAccessor* pAccessor) {
    const nn::font::detail::RuntimeTypeInfo* targetTypeInfo = T::GetRuntimeTypeInfoStatic();
    const nn::font::detail::RuntimeTypeInfo* typeInfo = pAccessor->GetRuntimeTypeInfo();

    while (typeInfo) {
        if (typeInfo == targetTypeInfo) {
            return static_cast<T*>(pAccessor);
        }

        typeInfo = typeInfo->m_ParentTypeInfo;
    }

    return nullptr;
}

inline nn::ui2d::ResourceAccessor* getResourceAccessor(const nn::ui2d::Layout* pLayout) {
    return *reinterpret_cast<nn::ui2d::ResourceAccessor* const*>(
        reinterpret_cast<const u8*>(pLayout) + 0x40);
}
}  // namespace

/**
 * Creates an empty layout keeper.
 */
LayoutKeeper::LayoutKeeper() = default;

/**
 * Reinitializes the shaders of the screen's layout resource.
 */
void LayoutKeeper::reinitializeShader() {
    if (!mScreen) {
        return;
    }

    nn::ui2d::Layout* layout = mScreen->mLayout;

    if (!layout) {
        return;
    }

    nn::ui2d::ResourceAccessor* accessor = getResourceAccessor(layout);

    if (!accessor) {
        return;
    }

    auto* resource = dynamicCastResourceAccessor<eui::MultiArcResourceAccessor>(accessor);

    if (!resource) {
        return;
    }

    static_cast<LayoutResource*>(resource)->reinitializeShaders(
        reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()));
}
}  // namespace al
