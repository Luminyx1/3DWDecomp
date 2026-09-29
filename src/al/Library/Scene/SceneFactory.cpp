#include "Library/Scene/SceneFunction.hpp"

namespace alSceneFunction {
    /** @brief Creates an empty scene factory. */
    SceneFactory::SceneFactory(const char* pName) : al::Factory<SceneFunction>(pName) {}

    /** @brief Returns the custom heap allocator for a scene; the base factory has none. */
    al::MemorySceneHeapCustomAlloc* SceneFactory::tryGetCustomAlloc(const char*) {
        return nullptr;
    }
};
