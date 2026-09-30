#include "Library/Draw/ViewRendererCreator.hpp"

#include "Library/Draw/ViewRenderer.hpp"

namespace al {

/**
 * Creates a view renderer.
 * @param pInfo Graphics system info.
 * @return The new view renderer.
 */
ViewRenderer* ViewRendererCreator::createViewRenderer(GraphicsSystemInfo* pInfo) {
    return new ViewRenderer(pInfo);
}

/**
 * Deletes a view renderer.
 * @param pRenderer View renderer, may be nullptr.
 */
void ViewRendererCreator::deleteViewRenderer(ViewRenderer* pRenderer) {
    delete pRenderer;
}

}  // namespace al
