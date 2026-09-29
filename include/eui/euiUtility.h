#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace nn::ui2d { class TextureInfo; }
namespace sead { class Heap; }

namespace eui {
sead::Heap* GetNwAllocatorHeap();
void ApplyTextureInfoToMaterial(nn::ui2d::Pane* pPane, const nn::ui2d::TextureInfo& rTexture, int index);
}
