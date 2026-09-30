#include "Library/Scene/SceneFunction.hpp"

namespace alSceneFunction {
/**
 * Constructs a scene factory.
 * @param pName Factory name.
 */
SceneFactory::SceneFactory(const char* pName) : Factory(pName) {}

/**
 * Gets the custom scene heap allocator of a scene.
 * @param pName Scene name.
 * @return Always nullptr.
 */
al::MemorySceneHeapCustomAlloc* SceneFactory::tryGetCustomAlloc(const char* pName) {
    return nullptr;
}
}  // namespace alSceneFunction
