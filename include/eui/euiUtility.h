#pragma once

#include <nn/ui2d/ui2d_Pane.h>
#include <prim/seadEnum.h>
#include <common/aglTextureEnum.h>

namespace nn::gfx { class DescriptorSlot; }
namespace nn::ui2d { class TextureInfo; }
namespace sead { class Heap; }
namespace agl { class TextureData; }
namespace agl::utl { class MultiFilter; }

namespace eui {
class LayoutEx;
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
