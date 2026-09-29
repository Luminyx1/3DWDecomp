#pragma once

namespace al {
class LiveActor;
class SkyboxDirector;
}  // namespace al

namespace SkyboxFunction {
al::SkyboxDirector* getSkyboxDirector(const al::LiveActor* pActor);
}  // namespace SkyboxFunction
