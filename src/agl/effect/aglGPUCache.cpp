#include "effect/aglGPUCache.h"

#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"

namespace agl {

/**
 * Inserts a command buffer barrier that invalidates every GPU cache.
 * @param pDrawContext draw context whose command buffer receives the barrier
 */
void GPUCache::invalidateAll(DrawContext* pDrawContext)
{
    nvnCommandBufferBarrier(pDrawContext->getNvnCommandBuffer(), 0xff);
}

}  // namespace agl
