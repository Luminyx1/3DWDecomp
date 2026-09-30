#pragma once

namespace al {
    class LightIntensityDirector;
    class LiveActor;
}  // namespace al

namespace LightIntensityFunction {
    al::LightIntensityDirector* getLightIntensityDirector(const al::LiveActor* pActor);
}
