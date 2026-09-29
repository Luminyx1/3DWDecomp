#include "Project/Se/SkyboxFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Project/Se/GraphicsSystemInfoView.hpp"

namespace SkyboxFunction {
/**
 * @brief Gets the skybox director of the scene an actor belongs to.
 * @param pActor The actor whose scene is queried.
 * @return The scene's skybox director.
 */
al::SkyboxDirector* getSkyboxDirector(const al::LiveActor* pActor) {
    return static_cast<al::GraphicsSystemInfoView*>(pActor->getSceneInfo()->_78)->mSkyboxDirector;
}
}  // namespace SkyboxFunction
