#pragma once

#include <basis/seadTypes.h>

#include "Library/Fog/FogParam.hpp"
#include "Library/Fog/YFogParam.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace sead {
class Camera;
}  // namespace sead

namespace al {
class GraphicsSystemInfo;
class LiveActor;
class Resource;

/**
 * @brief Keeps the requested distance fog and height fog parameters.
 */
class FogDirector {
public:
    FogDirector(GraphicsSystemInfo* pInfo);

    void initStageResource(const Resource* pResource, const char* pStageName);
    void endInit();
    void clear();
    void updateRequest();
    void updateCamera(const sead::Camera* pCamera);
    void requestFog(s32 priority, s32 step, const FogParam& rParam);
    void requestYFog(s32 priority, s32 step, const YFogParam& rParam);
    bool isUsingMulFog() const;
    bool isUsingMulYFog() const;

    const FogParam& getFogParam() const { return mFogKeeper.getCurrentParam(); }
    const YFogParam& getYFogParam() const { return mYFogKeeper.getCurrentParam(); }

private:
    void* _0;
    GraphicsParamRequestInterpKeeper<FogParam> mFogKeeper;
    GraphicsParamRequestInterpKeeper<YFogParam> mYFogKeeper;
};

}  // namespace al

namespace FogFunction {
al::FogDirector* getFogDirector(const al::LiveActor* pActor);
}  // namespace FogFunction
