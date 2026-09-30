#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"

#include <math/seadVector.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Shader/DeferredRendering/AtmosScatterCubeMap.hpp"
#include "Library/Shader/ForwardRendering/CubeMapKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"

namespace al {
/**
 * Constructs the cube map director without cube map sources.
 * @param pGraphicsSystemInfo Graphics system info.
 */
CubeMapDirector::CubeMapDirector(GraphicsSystemInfo* pGraphicsSystemInfo)
    : mGraphicsSystemInfo(pGraphicsSystemInfo) {}

/**
 * Destroys the cube map sources.
 */
CubeMapDirector::~CubeMapDirector() {
    if (mAtmosScatterCubeMap) {
        delete mAtmosScatterCubeMap;
        mAtmosScatterCubeMap = nullptr;
    }
    if (mShaderCubeMapKeeper) {
        delete mShaderCubeMapKeeper;
        mShaderCubeMapKeeper = nullptr;
    }
}

/**
 * Initializes the capture point cube maps from the stage resource.
 * @param pResource Stage resource.
 * @param pName Stage name.
 * @param pKit Live actor kit.
 */
void CubeMapDirector::initStageResource(const Resource* pResource, const char* pName,
                                        const LiveActorKit* pKit) {
    if (mShaderCubeMapKeeper) {
        mShaderCubeMapKeeper->initStageResource(pResource, pName, pKit);
    }
}

/**
 * Updates the atmospheric scattering cube map before drawing.
 */
void CubeMapDirector::preDrawGraphics() {
    if (mAtmosScatterCubeMap) {
        mAtmosScatterCubeMap->preDrawGraphics();
    }
}

/**
 * Finishes the initialization of the capture point cube maps.
 */
void CubeMapDirector::endInit() {
    if (mShaderCubeMapKeeper) {
        mShaderCubeMapKeeper->endInit();
    }
}

/**
 * Uses cube maps captured at capture points.
 * @param pPlayerHolder Player holder.
 */
void CubeMapDirector::initByCapturePoint(PlayerHolder* pPlayerHolder) {
    mShaderCubeMapKeeper = new ShaderCubeMapKeeper(mGraphicsSystemInfo, pPlayerHolder);
}

/**
 * Uses a cube map rendered from the atmospheric scattering.
 */
void CubeMapDirector::initByAtmosScatter() {
    if (mAtmosScatterCubeMap) {
        return;
    }
    mAtmosScatterCubeMap = new AtmosScatterCubeMap(mGraphicsSystemInfo);
}

/**
 * Activates the cube map textures.
 * @return Whether a cube map was activated.
 */
bool CubeMapDirector::activateCubeMapTexture(s32 a, s32 b, s32 c, bool d) const {
    if (mAtmosScatterCubeMap) {
        return mAtmosScatterCubeMap->activateCubeMapTexture(b, d);
    }
    if (mShaderCubeMapKeeper) {
        return mShaderCubeMapKeeper->activateCubeMapTexture(a, b, c, d);
    }
    return false;
}

/**
 * Checks whether a capture point cube map is drawn.
 * @return Whether a capture point cube map is drawn.
 */
bool CubeMapDirector::isDrawCapturePointCubeMap() const {
    if (mShaderCubeMapKeeper) {
        return mShaderCubeMapKeeper->isDrawCubeMap();
    }
    return false;
}

/**
 * Renders the atmospheric scattering cube map.
 * @param shaderMode Current shader mode.
 * @return The shader mode after rendering.
 */
agl::ShaderMode CubeMapDirector::renderToCubeMap(agl::ShaderMode shaderMode) const {
    if (mAtmosScatterCubeMap && mGraphicsSystemInfo &&
        (mGraphicsSystemInfo->_40 == 1 || mGraphicsSystemInfo->_40 == 2)) {
        return mAtmosScatterCubeMap->renderToCubeMap(shaderMode);
    }
    return shaderMode;
}

/**
 * Gets the irradiance cube map sampler.
 * @param index Cube map index.
 * @return The irradiance sampler.
 */
const agl::TextureSampler* CubeMapDirector::getIrradianceSampler(s32 index) const {
    if (mAtmosScatterCubeMap) {
        return mAtmosScatterCubeMap->getIrradianceSampler(index);
    }
    if (mShaderCubeMapKeeper) {
        return mShaderCubeMapKeeper->getIrradiance(index, sead::Vector3f::zero);
    }
    return getBlackCubeSampler();
}

/**
 * Gets the mirror cube map sampler.
 * @param index Cube map index.
 * @return The mirror sampler.
 */
const agl::TextureSampler* CubeMapDirector::getCubeMapMirrorSampler(s32 index) const {
    if (mAtmosScatterCubeMap) {
        return mAtmosScatterCubeMap->getCubeMapMirrorSampler(index);
    }
    if (mShaderCubeMapKeeper) {
        return mShaderCubeMapKeeper->getRoughnessCubeMap(0, index);
    }
    return getBlackCubeSampler();
}
}  // namespace al
