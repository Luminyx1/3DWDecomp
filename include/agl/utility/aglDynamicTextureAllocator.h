#pragma once

#include <basis/seadTypes.h>
#include <heap/seadDisposer.h>
#include <prim/seadSafeString.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglTextureEnum.h"

namespace agl {
class DrawContext;
class TextureData;
}  // namespace agl

namespace agl::utl {

// TODO: layout and remaining members
class DynamicTextureAllocator {
    SEAD_SINGLETON_DISPOSER(DynamicTextureAllocator)

public:
    enum AllocateType {
        cAllocateType_0 = 0,
    };

    TextureData* allocMultiSampleWithoutContext(DrawContext* pDrawContext,
                                                const sead::SafeString& rName,
                                                TextureFormat format, u32 width, u32 height,
                                                MultiSampleType multiSample,
                                                GPUMemVoidAddr* pAddr, AllocateType type,
                                                bool b1, bool b2);
    void free(const TextureData* pTexture);
};

}  // namespace agl::utl
