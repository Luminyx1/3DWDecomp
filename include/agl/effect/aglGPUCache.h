#pragma once

namespace agl {

class DrawContext;

class GPUCache {
public:
    static void invalidateAll(DrawContext* pDrawContext);
};

}  // namespace agl
