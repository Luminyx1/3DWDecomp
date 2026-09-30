#include "Library/Memory/SceneHeapSetter.hpp"

#include "Library/System/SystemKit.hpp"
#include "Project/Memory/MemorySystem.hpp"

namespace al {
/**
 * Sets the scene heap as current heap for the lifetime of the setter.
 */
SceneHeapSetter::SceneHeapSetter()
    : mSetter(alProjectInterface::getSystemKit()->getMemorySystem()->getSceneHeap()),
      mSceneHeap(alProjectInterface::getSystemKit()->getMemorySystem()->getSceneHeap()) {}
}  // namespace al
