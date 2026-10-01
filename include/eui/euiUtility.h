#pragma once

#include <nn/ui2d/ui2d_Pane.h>
#include <prim/seadEnum.h>
#include <common/aglTextureEnum.h>
#include <math/seadVector.h>
#include <prim/seadStringBuilder.h>

namespace nn::gfx { class DescriptorSlot; }
namespace nn::ui2d { class TextureInfo; struct ResExtUserData; struct ResExtUserDataList; }
namespace sead { class Heap; }
namespace agl { class TextureData; }
namespace agl::utl { class MultiFilter; }

namespace eui {
class LayoutEx;

template <typename T, typename U>
T* DynamicCast(U* pObj) {
    const auto* pType = T::GetRuntimeTypeInfoStatic();

    if (pObj != nullptr) {
        for (auto* type = pObj->GetRuntimeTypeInfo(); type != nullptr; type = type->m_ParentTypeInfo) {
            if (type == pType) {
                return static_cast<T*>(pObj);
            }
        }
    }

    return nullptr;
}

void CreateLayoutItemUniqueName(sead::StringBuilder* pName, const char* pItem, const LayoutEx* pLayout);
void CreateLayoutItemUniqueNameByPath(sead::StringBuilder* pName, const char* pPath, const LayoutEx* pLayout);
// Draw targets are passed by value as a four-byte index.
class DrawTarget {
public:
    // index identifies one of the two display targets.
    explicit DrawTarget(int index) : mIndex(index) {}
    operator int() const volatile { return mIndex; }
private:
    int mIndex;
};

const nn::ui2d::ResExtUserData* FindExtUserDataFromList(const nn::ui2d::ResExtUserDataList* pList, const char* pName);
void AdjustPaneSizeToTextSize(nn::ui2d::Pane* pPane, LayoutEx* pLayout);
void CenteringPanePair(nn::ui2d::Pane* pPane);
void ApplyCaptureUse(nn::ui2d::Pane* pPane, LayoutEx* pLayout);
void ApplyDynamicCaptureUse(nn::ui2d::Pane* pPane, LayoutEx* pLayout);
void SetupPaneAfterBuild(nn::ui2d::Pane* pPane, LayoutEx* pLayout);
void IteratePaneForSetupPaneAfterBuild(nn::ui2d::Pane* pPane, LayoutEx* pLayout);
LayoutEx* FindHitLayout(const sead::Vector2f& rPosition, LayoutEx* pLayout);
LayoutEx* FindHitLayoutRecursive_(const sead::Vector2f& rPosition, LayoutEx* pHit, LayoutEx* pLayout, const nn::ui2d::Pane* pPane);
bool IsHitPane(const sead::Vector2f& rPosition, const nn::ui2d::Pane* pPane);
agl::utl::MultiFilter* InitializeMultiFilter(sead::Heap* pHeap,
    const nn::ui2d::Pane& rPane, LayoutEx* pLayout);
SEAD_ENUM(Direction, cUp, cDown, cLeft, cRight)

Direction GetOppositeDirection(Direction direction);
f32 GetRadAngleOfDirection(Direction direction);
bool RegisterSlotForTexture(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::TextureView& rView,
                            void* pUserData);
bool RegisterSlotForSampler(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::Sampler& rSampler,
                            void* pUserData);
void UnregisterSlotForTexture(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::TextureView& rView,
                              void* pUserData);
void UnregisterSlotForSampler(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::Sampler& rSampler,
                              void* pUserData);
sead::Heap* GetNwAllocatorHeap();
void ApplyTextureInfoToMaterial(nn::ui2d::Pane* pPane, const nn::ui2d::TextureInfo& rTexture, int index);
void SetupTextureInfoByAglTextureData(nn::ui2d::TextureInfo* pInfo,
    const agl::TextureData& rTexture, const agl::TextureCompSel* pComponentSelection);
void SetupAglTextureDataByTextureInfo(agl::TextureData* pTexture,
    const nn::ui2d::TextureInfo& rInfo);
}
