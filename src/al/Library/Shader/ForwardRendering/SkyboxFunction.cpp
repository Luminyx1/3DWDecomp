#include "Library/Shader/ForwardRendering/SkyboxFunction.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"

namespace SkyboxFunction {
    /**
     * Gets the scene's skybox director.
     * @param pActor Actor of the scene.
     * @return The skybox director.
     */
    al::SkyboxDirector* getSkyboxDirector(const al::LiveActor* pActor) {
        return static_cast<al::GraphicsSystemInfo*>(pActor->getSceneInfo()->_78)->mSkyboxDirector;
    }
}  // namespace SkyboxFunction
