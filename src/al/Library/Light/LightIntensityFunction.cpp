#include "Library/Light/LightIntensityFunction.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"

namespace LightIntensityFunction {
    /**
     * Gets the scene's light intensity director.
     * @param pActor Actor of the scene.
     * @return The light intensity director.
     */
    al::LightIntensityDirector* getLightIntensityDirector(const al::LiveActor* pActor) {
        return pActor->getSceneInfo()->graphicsSystemInfo->getLightIntensityDirector();
    }
}  // namespace LightIntensityFunction
