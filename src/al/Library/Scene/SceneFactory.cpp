#include "Library/Scene/SceneFunction.hpp"

namespace alSceneFunction {
    /**
     * @brief Creates a scene factory with no entries.
     * @param pName The name of the factory.
     */
    SceneFactory::SceneFactory(const char* pName) : al::Factory<SceneFunction>(pName) {}

    /**
     * @brief Gets the custom heap allocator for a scene.
     * @param pSceneName The name of the scene (unused by the base factory).
     * @return The allocator to use, or nullptr for the default; the base factory always returns nullptr.
     */
    al::MemorySceneHeapCustomAlloc* SceneFactory::tryGetCustomAlloc(const char* pSceneName) {
        return nullptr;
    }
};
