#include "Library/Shader/DeferredRendering/AtmosScatterCubeMap.hpp"

#include "common/aglDrawContext.h"
#include "common/aglShaderLocation.h"
#include "environment/aglCubeMap.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Shader/DeferredRendering/AtmosScatter.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDrawInfo.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDrawer.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"

namespace {
using namespace al;

NERVE_DECL(AtmosScatterCubeMap, Initial)
NERVE_DECL(AtmosScatterCubeMap, FlipCubeMap)
NERVE_DECL(AtmosScatterCubeMap, DrawFace)
NERVE_DECL(AtmosScatterCubeMap, DrawRoughness)

NERVES_MAKE_NOSTRUCT(AtmosScatterCubeMap, Initial, FlipCubeMap, DrawFace, DrawRoughness)

struct MipMapParam {
    u32 mCount;
    f32 mSigma;
};

const MipMapParam cMipMapParam[] = {
    {1, 1.5f}, {2, 1.5f}, {2, 1.5f}, {2, 1.5f}, {2, 1.5f}, {1, 1.0f}, {1, 1.0f}, {1, 1.0f},
    {1, 1.0f}, {1, 1.0f}, {1, 1.0f}, {1, 1.0f}, {1, 1.0f}, {1, 1.0f}, {1, 1.0f},
};
}  // namespace

namespace al {
/**
 * Creates the double buffered cube maps rendered from the atmospheric scattering.
 * @param pGraphicsSystemInfo Graphics system info.
 */
AtmosScatterCubeMap::AtmosScatterCubeMap(GraphicsSystemInfo* pGraphicsSystemInfo)
    : NerveExecutor("大気散乱キューブマップ"), mGraphicsSystemInfo(pGraphicsSystemInfo),
      mAtmosScatter(pGraphicsSystemInfo->mAtmosScatter), _20(pGraphicsSystemInfo->_d60) {
    initNerve(&NrvAtmosScatterCubeMapInitial, 0);
    mCubeMaps.pushBack(new agl::env::CubeMap());
    mCubeMaps.pushBack(new agl::env::CubeMap());
    u32 mipLevelNum = calcMaxMipLevelNum(0x80);
    mCubeMaps[0]->initialize(getCurrentHeap(), agl::TextureFormat(0x1d), 0x80, mipLevelNum);
    mCubeMaps[1]->initialize(getCurrentHeap(), agl::TextureFormat(0x1d), 0x80, mipLevelNum);
    mDrawInfo = new CubeMapDrawInfo(pGraphicsSystemInfo);
    mIrradianceSampler.applyTextureData(getBlackCubeTexture());
    mMirrorSampler.applyTextureData(getBlackCubeTexture());
}

/**
 * Updates the cube map rendering state.
 */
void AtmosScatterCubeMap::preDrawGraphics() {
    updateNerve();
}

/**
 * Starts rendering the cube map faces.
 */
void AtmosScatterCubeMap::exeInitial() {
    setNerve(this, &NrvAtmosScatterCubeMapDrawFace);
}

/**
 * Renders one cube map face per frame.
 */
void AtmosScatterCubeMap::exeDrawFace() {
    if (mFaceIndex++ >= 5) {
        setNerve(this, &NrvAtmosScatterCubeMapDrawRoughness);
    }
}

/**
 * Generates the roughness mip maps.
 */
void AtmosScatterCubeMap::exeDrawRoughness() {
    mFaceIndex = -1;

    if (isGreaterStep(this, 2)) {
        setNerve(this, &NrvAtmosScatterCubeMapFlipCubeMap);
    }
}

/**
 * Swaps the rendered cube map with the displayed one.
 */
void AtmosScatterCubeMap::exeFlipCubeMap() {
    mCubeMapIndex = mCubeMapIndex == 0;
    mIrradianceSampler.applyTextureData(mCubeMaps[mCubeMapIndex == 0]->getTextureData());
    mIrradianceSampler.setMinLod(5.0f);
    mMirrorSampler.applyTextureData(mCubeMaps[1 - mCubeMapIndex]->getTextureData());
    mMirrorSampler.setMinLod(0.0f);
    setNerve(this, &NrvAtmosScatterCubeMapDrawFace);
}

/**
 * Renders the current cube map face or its mip maps.
 * @param shaderMode Current shader mode.
 * @return The shader mode after rendering.
 */
agl::ShaderMode AtmosScatterCubeMap::renderToCubeMap(agl::ShaderMode shaderMode) const {
    if (mFaceIndex >= 0 && isNerve(this, &NrvAtmosScatterCubeMapDrawFace)) {
        s32 face = mFaceIndex;
        {
            CubeMapDrawer drawer(mDrawInfo, &shaderMode, mCubeMaps[mCubeMapIndex], face, 100.0f,
                                 100000.0f, mCubeMapPos, 0, true);
            if (mAtmosScatter) {
                shaderMode = mAtmosScatter->drawFarToCubeMap(
                    mGraphicsSystemInfo->_60 + face, drawer.getViewMatrix(), drawer.getProjMatrix(),
                    sead::Vector2f::zero, drawer.getFovy(), drawer.getAspect(), shaderMode);
            }
        }

        return shaderMode;
    }

    if (isNerve(this, &NrvAtmosScatterCubeMapDrawRoughness)) {
        agl::env::CubeMap* cubeMap = mCubeMaps[mCubeMapIndex];
        agl::DrawContext* drawContext =
            reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext);
        cubeMap->begin(drawContext, false, true);
        u32 mipLevelNum = mCubeMaps[0]->getTextureData().getMipLevelNum();

        if (isFirstStep(this)) {
            cubeMap->generateMipMap(
                reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), 1, 2,
                cMipMapParam[0].mCount, cMipMapParam[0].mSigma, true);
        } else if (isStep(this, 1)) {
            cubeMap->generateMipMap(
                reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), 2, 3,
                cMipMapParam[1].mCount, cMipMapParam[1].mSigma, true);
            cubeMap->generateMipMap(
                reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), 3, 4,
                cMipMapParam[2].mCount, cMipMapParam[2].mSigma, true);
        } else {
            for (u32 i = 4; i < mipLevelNum; i++) {
                cubeMap->generateMipMap(
                    reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), i,
                    i + 1, cMipMapParam[i - 1].mCount, cMipMapParam[i - 1].mSigma, true);
            }
        }

        cubeMap->end(reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext));
    }

    return shaderMode;
}

/**
 * Gets the irradiance cube map sampler.
 * @param index Unused cube map index.
 * @return The irradiance sampler.
 */
const agl::TextureSampler* AtmosScatterCubeMap::getIrradianceSampler(s32 index) const {
    return &mIrradianceSampler;
}

/**
 * Gets the mirror cube map sampler.
 * @param index Unused cube map index.
 * @return The mirror sampler.
 */
const agl::TextureSampler* AtmosScatterCubeMap::getCubeMapMirrorSampler(s32 index) const {
    return &mMirrorSampler;
}

/**
 * Activates the cube map sampler for a sampler slot.
 * @param type Sampler slot type.
 * @param isRefract Whether the refraction roughness location is used.
 * @return Always true.
 */
bool AtmosScatterCubeMap::activateCubeMapTexture(s32 type, bool isRefract) const {
    const agl::SamplerLocation* location = &getSamplerLocationCubeMapRoughness();
    const agl::TextureSampler* sampler;

    if (type == 5) {
        location = &getSamplerLocationCubeMapIrradiance();
        sampler = &mIrradianceSampler;
    } else {
        if (isRefract) {
            location = &getSamplerLocationCubeMapRoughnessRefract();
        }

        sampler = &mMirrorSampler;
    }

    sampler->activate(reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext),
                      *location, -1, false);
    return true;
}

/**
 * Destroys the cube map samplers.
 */
AtmosScatterCubeMap::~AtmosScatterCubeMap() = default;
}  // namespace al
