#include "Library/Play/Graphics/NoiseTextureKeeper.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadGraphicsContext.h>
#include <math/seadVector.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDrawInfo.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"
#include "Library/Texture/TextureUtil.hpp"

namespace al {
// TODO: Move to their own headers once their layout is known; only the members used here are known.
class GraphicsRenderInfo {
public:
    agl::DrawContext* mDrawContext;
};

class GraphicsUpdateInfo {
public:
    f32 mDeltaTime;
};
}  // namespace al

namespace {
const al::UniformBlockLayout cNoiseCommonUboLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 1}, {1, agl::UniformBlock::cType_Vec4, 1},
    {2, agl::UniformBlock::cType_Vec4, 1}, {3, agl::UniformBlock::cType_Vec4, 1},
    {4, agl::UniformBlock::cType_Vec4, 1}, {5, agl::UniformBlock::cType_Vec4, 1},
    {6, agl::UniformBlock::cType_Vec2, 1}, {7, agl::UniformBlock::cType_Vec4, 1},
};
}  // namespace

/**
 * @brief Parameters of a noise texture, uploaded to the NoiseCommonUbo uniform block.
 */
struct NoiseTextureParam {
    sead::Vector3f mScale;
    sead::Vector4f mParam0 = {5.0f, 5.0f, 0.0f, 0.0f};
    sead::Vector4f mParam1 = sead::Vector4f::zero;
    sead::Vector4f mParam2 = sead::Vector4f::zero;
    sead::Vector2f mParam3 = sead::Vector2f::zero;
    sead::Vector4f mParam4 = sead::Vector4f::zero;
    f32 mTime = 0.0f;
    f32 mTimeSpeed = 1.0f;
};

/**
 * @brief A procedurally generated noise texture with its parameter uniform block.
 */
class NoiseTexture {
public:
    /**
     * Creates the texture unit and the parameter uniform block.
     * @param pName Name of the texture.
     */
    NoiseTexture(const char* pName)
        : mTextureUnit(new al::TextureUnit(pName)),
          mUniformBlock(al::createUniformBlock(cNoiseCommonUboLayout, 8, nullptr, 2)) {}

    /**
     * Destroys the texture unit and the uniform block.
     */
    void finalize() {
        al::TextureUnit* textureUnit = mTextureUnit;

        if (textureUnit != nullptr) {
            textureUnit->finalize();
            delete textureUnit;
            mTextureUnit = nullptr;
        }

        if (mUniformBlock != nullptr) {
            delete mUniformBlock;
            mUniformBlock = nullptr;
        }
    }

    /**
     * Creates the 2D or 3D texture image.
     * @param width Width in texels.
     * @param height Height in texels.
     * @param depth Depth in texels, 0 for a 2D texture.
     * @param format Texture format.
     * @param rCompSel Component selection of the sampler.
     */
    void create(s32 width, s32 height, s32 depth, agl::TextureFormat format,
                const al::CompSelType& rCompSel) {
        s32 mipLevelNum = al::calcMaxMipLevelNum(width);

        al::TextureInitArg arg;
        arg.mIsCubemap = false;
        arg.mFormat = format;
        arg.mWrapX = 1;
        arg.mWrapY = 1;
        arg.mWrapZ = 1;
        arg.mMagFilter = 1;
        arg.mMinFilter = 1;
        arg.mMipFilter = 2;
        arg.mDepth = depth;
        arg.mCompSel = rCompSel;
        arg.mWidth = width;
        arg.mHeight = height;
        arg.mMipLevelNum = mipLevelNum;

        if (mTextureUnit->tryCreateTexture(arg)) {
            mIsUpdated = true;
        }
    }

    /**
     * Creates the cubemap texture image.
     * @param size Width and height of a face in texels.
     * @param format Texture format.
     * @param rCompSel Component selection of the sampler.
     */
    void createCubemap(s32 size, agl::TextureFormat format, const al::CompSelType& rCompSel) {
        s32 mipLevelNum = al::calcMaxMipLevelNum(size);

        al::TextureInitArg arg;
        arg.mFormat = format;
        arg.mWrapX = 1;
        arg.mWrapY = 1;
        arg.mWrapZ = 1;
        arg.mMagFilter = 1;
        arg.mMinFilter = 1;
        arg.mMipFilter = 2;
        arg.mWidth = size;
        arg.mHeight = size;
        arg.mDepth = 6;
        arg.mMipLevelNum = mipLevelNum;
        arg.mIsCubemap = true;
        arg.mCompSel = rCompSel;

        if (mTextureUnit->tryCreateTexture(arg)) {
            mIsUpdated = true;
        }
    }

    /**
     * Checks whether the texture was created.
     * @return True if the texture was created.
     */
    bool isValid() const { return mTextureUnit->getSampler() != nullptr; }

    /**
     * Checks whether the texture has to be rendered this frame.
     * @return True if the texture has to be rendered.
     */
    bool isNeedDraw() const { return isValid() && mIsNeedDraw; }

    /**
     * Gets the sampler of the texture.
     * @return Sampler of the texture.
     */
    const agl::TextureSampler* getSampler() const { return mTextureUnit->getSampler(); }

    /**
     * Copies the noise parameters of another texture and marks them as updated.
     * @param pOther Texture to copy the parameters from.
     */
    void copyParam(const NoiseTexture* pOther) {
        mParam.mScale = pOther->mParam.mScale;
        mParam.mParam0 = pOther->mParam.mParam0;
        mParam.mParam1 = pOther->mParam.mParam1;
        mParam.mParam2 = pOther->mParam.mParam2;
        mParam.mParam3 = pOther->mParam.mParam3;
        mParam.mParam4 = pOther->mParam.mParam4;
        mParam.mTime = pOther->mParam.mTime;
        mParam.mTimeSpeed = pOther->mParam.mTimeSpeed;
        mIsUpdated = true;
    }

    /**
     * Schedules the rendering of updated parameters and advances the time.
     * @param deltaTime Elapsed time since the last frame.
     */
    void update(f32 deltaTime) {
        if (mIsUpdated) {
            mIsNeedDraw = true;
            mIsUpdated = false;
        } else {
            mIsNeedDraw = false;
        }

        mParam.mTime += deltaTime * mParam.mTimeSpeed;
        updateUbo();
    }

    void updateUbo();

    /**
     * Clears the texture.
     * @param pDrawContext Draw context.
     */
    void clear(agl::DrawContext* pDrawContext) const {
        agl::RenderBuffer renderBuffer;

        if (mTextureUnit->is3D()) {
            mTextureUnit->getDepth();
        } else {
            mTextureUnit->is2D();
        }

        mTextureUnit->getTextureData()->generateMipMap(pDrawContext);
    }

    /**
     * Renders the noise into the texture if its parameters were updated.
     * @param pDrawContext Draw context.
     * @param pProgram Shader program rendering the noise.
     * @param pFullScreenTriangle Full screen triangle to render with.
     * @param pName Name of the draw pass.
     * @param pCubeMapDrawInfo Cubemap draw info, used by cubemap textures.
     */
    void drawToTexture(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                       al::FullScreenTriangle* pFullScreenTriangle, const char* pName,
                       al::CubeMapDrawInfo* pCubeMapDrawInfo) {
        if (!isNeedDraw()) {
            return;
        }

        pProgram->activate(pDrawContext, true);
        al::setUniformBlockToShader(mUniformBlock, pDrawContext, *pProgram, "NoiseCommonUbo", 0);
        agl::RenderBuffer renderBuffer;

        if (!mTextureUnit->isCubemap() && mTextureUnit->is3D()) {
            mTextureUnit->getDepth();
        }

        mTextureUnit->getTextureData()->generateMipMap(pDrawContext);
    }

    al::TextureUnit* mTextureUnit;
    al::UniformBlock* mUniformBlock;
    NoiseTextureParam mParam;
    bool mIsUpdated = true;
    bool mIsNeedDraw = true;
};

static_assert(sizeof(NoiseTexture) == 0x70);

namespace al {

/**
 * Creates every noise texture and looks up the noise shaders.
 * @param pInfo Graphics system info.
 * @param pShaderHolder Shader holder used to look up the noise shaders.
 */
NoiseTextureKeeper::NoiseTextureKeeper(GraphicsSystemInfo* pInfo, ShaderHolder* pShaderHolder)
    : PartsGraphics(pInfo), mBlack2D(new NoiseTexture("Black2D")),
      mBlack3D(new NoiseTexture("Black3D")),
      mCurlShader(pShaderHolder->tryGetShaderProgram("alNoiseCurl")),
      mCurl2D(new NoiseTexture("Curl")), mCurl3D(new NoiseTexture("Curl3D")),
      mSimpleShader(pShaderHolder->tryGetShaderProgram("alNoiseSimple")),
      mSimple(new NoiseTexture("Simple")),
      mPerlinShader(pShaderHolder->tryGetShaderProgram("alNoisePerlin")),
      mPerlin2D(new NoiseTexture("Perlin2D")), mPerlin3D(new NoiseTexture("Perlin3D")),
      mPerlinFbm2D(new NoiseTexture("PerlinFbm2D")), mPerlinFbm3D(new NoiseTexture("PerlinFbm3D")),
      mRidge3D(new NoiseTexture("Ridge3D")),
      mWorleyShader(pShaderHolder->tryGetShaderProgram("alNoiseWorley")),
      mWorley2D(new NoiseTexture("Worley2D")), mWorley3D(new NoiseTexture("Worley3D")),
      mWorleyThin3D(new NoiseTexture("WorleyThin3D")),
      mWorleyThinAnim3D(new NoiseTexture("WorleyThinAnim3D")),
      mCloud3D(new NoiseTexture("Cloud 3D")),
      mCloudLikeFbmShader(pShaderHolder->tryGetShaderProgram("alNoiseCloudLikeFbm")),
      mCloudLikeFbm3D(new NoiseTexture("CloudLikeFbm3D")),
      mOceanFoam3D(new NoiseTexture("OceanFoam3D")),
      mSnowCovered3D(new NoiseTexture("SnowCovered3D")), mFrost2D(new NoiseTexture("Frost2D")),
      mFrost3D(new NoiseTexture("Frost3D")), mCubeMapDrawInfo(new CubeMapDrawInfo(pInfo)),
      mGemShader(pShaderHolder->tryGetShaderProgram("alNoiseGem")),
      mGemNoiseCubemap(new NoiseTexture("GemNoiseCubemap")),
      mGemNoise3D(new NoiseTexture("GemNoise3D")),
      mCausticsShader(pShaderHolder->tryGetShaderProgram("alNoiseCaustics")),
      mCaustics3D(new NoiseTexture("Caustics3D")),
      mCaustics3D1ch(new NoiseTexture("Caustics3D1ch")),
      mFullScreenTriangle(pInfo->mFullScreenTriangle), mTime(0.0f), mDeltaTime(0.0f) {
    mTextures.pushBack(mBlack2D);
    mTextures.pushBack(mBlack3D);
    mTextures.pushBack(mCurl2D);
    mTextures.pushBack(mCurl3D);
    mTextures.pushBack(mSimple);
    mTextures.pushBack(mWorley2D);
    mTextures.pushBack(mWorley3D);
    mTextures.pushBack(mWorleyThin3D);
    mTextures.pushBack(mWorleyThinAnim3D);
    mTextures.pushBack(mCloud3D);
    mTextures.pushBack(mCloudLikeFbm3D);
    mTextures.pushBack(mOceanFoam3D);
    mTextures.pushBack(mSnowCovered3D);
    mTextures.pushBack(mFrost2D);
    mTextures.pushBack(mFrost3D);
    mTextures.pushBack(mPerlin2D);
    mTextures.pushBack(mPerlin3D);
    mTextures.pushBack(mPerlinFbm2D);
    mTextures.pushBack(mPerlinFbm3D);
    mTextures.pushBack(mRidge3D);
    mTextures.pushBack(mGemNoiseCubemap);
    mTextures.pushBack(mGemNoise3D);
    mTextures.pushBack(mCaustics3D);
    mTextures.pushBack(mCaustics3D1ch);
}

/**
 * Destroys the noise textures.
 */
NoiseTextureKeeper::~NoiseTextureKeeper() {
    NoiseTextureKeeper::finalize();
}

/**
 * Destroys the texture and uniform block of every noise texture.
 */
void NoiseTextureKeeper::finalize() {
    s32 size = mTextures.size();

    for (s32 i = 0; i < size; i++) {
        mTextures[i]->finalize();
    }
}

/**
 * Creates the black fallback textures and the always used noise textures, then uploads the
 * parameters of every noise texture.
 */
void NoiseTextureKeeper::endInit() {
    mBlack2D->create(2, 2, 0, agl::TextureFormat::cTextureFormat_R8_uNorm, 0);
    mBlack3D->create(2, 2, 2, agl::TextureFormat::cTextureFormat_R8_uNorm, 0);

    declareUsingGemNoiseCubemapTexture();
    declareUsingPerlinFbmNoise2DTexture();
    declareUsingWorleyNoise2DTexture();
    declareUsingFrostNoise2DTexture();
    declareUsingCurlNoise2DTexture();
    declareUsingPerlinNoise2DTexture();
    declareUsingFrostNoise3DTexture();
    declareUsingWorleyNoise3DTexture();
    declareUsingWorleyThinNoise3DTexture();
    declareUsingWorleyThinAnimNoise3DTexture();
    declareUsingCloudLikeFbm3DNoiseTexture();
    declareUsingOceanFoam3DNoiseTexture();
    declareUsingSnowCovered3DNoiseTexture();
    declareUsingCurlNoise3DTexture();
    declareUsingRidgeNoise3DTexture();
    declareUsingCaustics3DTexture();
    declareUsingPerlinNoise3DTexture();
    declareUsingPerlinFbmNoise3DTexture();
    declareUsingGemNoise3DTexture();

    s32 size = mTextures.size();

    for (s32 i = 0; i < size; i++) {
        mTextures[i]->updateUbo();
    }
}

/**
 * Creates the gem noise cubemap texture.
 */
void NoiseTextureKeeper::declareUsingGemNoiseCubemapTexture() {
    mGemNoiseCubemap->createCubemap(128, agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 0);
    mGemNoiseCubemap->mParam.mScale.set(2.0f, 1.0f, 1.0f);
    mGemNoiseCubemap->mParam.mParam0.set(0.05f, 10.0f, 1.0f, 1.0f);
    mGemNoiseCubemap->mParam.mParam1.set(6.15f, 0.0f, 0.0f, 0.0f);
}

/**
 * Creates the 2D perlin fbm noise texture.
 */
void NoiseTextureKeeper::declareUsingPerlinFbmNoise2DTexture() {
    mPerlinFbm2D->create(256, 256, 0, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mPerlinFbm2D->mParam.mScale.set(2.0f, 2.0f, 1.0f);
    mPerlinFbm2D->mParam.mParam0.set(0.4f, 1.0f, 0.6f, 0.0f);
}

/**
 * Creates the 2D worley noise texture.
 */
void NoiseTextureKeeper::declareUsingWorleyNoise2DTexture() {
    mWorley2D->create(256, 256, 0, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mWorley2D->mParam.mScale.set(3.0f, 3.0f, 1.0f);
    mWorley2D->mParam.mParam0.set(0.0f, 1.0f, 0.0f, 0.0f);
}

/**
 * Creates the 2D frost noise texture.
 */
void NoiseTextureKeeper::declareUsingFrostNoise2DTexture() {
    mFrost2D->create(256, 256, 0, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mFrost2D->mParam.mScale.set(2.0f, 2.0f, 1.0f);
    mFrost2D->mParam.mParam0.set(0.4f, 1.0f, 0.6f, 0.0f);
}

/**
 * Creates the 2D curl noise texture.
 */
void NoiseTextureKeeper::declareUsingCurlNoise2DTexture() {
    mCurl2D->create(256, 256, 0, agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 0);
    mCurl2D->mParam.mScale.set(4.0f, 4.0f, 4.0f);
    mCurl2D->mParam.mParam0.set(0.5f, 0.5f, 0.0f, 0.0f);
}

/**
 * Creates the 2D perlin noise texture.
 */
void NoiseTextureKeeper::declareUsingPerlinNoise2DTexture() {
    mPerlin2D->create(256, 256, 0, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mPerlin2D->mParam.mScale.set(2.0f, 2.0f, 1.0f);
    mPerlin2D->mParam.mParam0.set(0.4f, 1.0f, 0.6f, 0.0f);
}

/**
 * Creates the 3D frost noise texture.
 */
void NoiseTextureKeeper::declareUsingFrostNoise3DTexture() {
    mFrost3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mFrost3D->mParam.mScale.set(2.0f, 2.0f, 1.0f);
    mFrost3D->mParam.mParam0.set(0.4f, 1.0f, 0.6f, 0.0f);
}

/**
 * Creates the 3D worley noise texture.
 */
void NoiseTextureKeeper::declareUsingWorleyNoise3DTexture() {
    mWorley3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mWorley3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mWorley3D->mParam.mParam0.set(0.0f, 1.0f, 0.0f, 0.0f);
}

/**
 * Creates the 3D thin worley noise texture.
 */
void NoiseTextureKeeper::declareUsingWorleyThinNoise3DTexture() {
    mWorleyThin3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mWorleyThin3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mWorleyThin3D->mParam.mParam0.set(0.0f, 1.0f, 0.0f, 0.0f);
    mWorleyThin3D->mParam.mParam3.set(0.57f, 2.23f);
    mWorleyThin3D->mParam.mParam4.set(0.2f, 1.3f, -0.1f, 0.0f);
}

/**
 * Creates the animated 3D thin worley noise texture.
 */
void NoiseTextureKeeper::declareUsingWorleyThinAnimNoise3DTexture() {
    mWorleyThinAnim3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mWorleyThinAnim3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mWorleyThinAnim3D->mParam.mParam0.set(0.0f, 1.0f, 0.0f, 0.0f);
    mWorleyThinAnim3D->mParam.mParam3.set(1.0f, 1.0f);
    mWorleyThinAnim3D->mParam.mParam4.set(1.0f, 5.0f, -0.5f, 0.0f);
}

/**
 * Creates the 3D cloud like fbm noise texture.
 */
void NoiseTextureKeeper::declareUsingCloudLikeFbm3DNoiseTexture() {
    mCloudLikeFbm3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mCloudLikeFbm3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mCloudLikeFbm3D->mParam.mParam0.set(1.0f, 1.0f, 1.0f, 1.0f);
}

/**
 * Creates the 3D ocean foam noise texture.
 */
void NoiseTextureKeeper::declareUsingOceanFoam3DNoiseTexture() {
    mOceanFoam3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mOceanFoam3D->mParam.mScale.set(4.0f, 4.0f, 4.0f);
    mOceanFoam3D->mParam.mParam0.set(0.0f, 0.25f, 1.0f, 1.0f);
    mOceanFoam3D->mParam.mParam1.set(0.25f, 1.0f, 0.0f, 0.0f);
}

/**
 * Creates the 3D snow covered noise texture.
 */
void NoiseTextureKeeper::declareUsingSnowCovered3DNoiseTexture() {
    mSnowCovered3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mSnowCovered3D->mParam.mScale.set(4.0f, 4.0f, 4.0f);
    mSnowCovered3D->mParam.mParam0.set(0.0f, 0.25f, 1.0f, 1.0f);
    mSnowCovered3D->mParam.mParam1.set(0.25f, 1.0f, 0.0f, 0.0f);
}

/**
 * Creates the 3D curl noise texture.
 */
void NoiseTextureKeeper::declareUsingCurlNoise3DTexture() {
    mCurl3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 0);
    mCurl3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mCurl3D->mParam.mParam0.set(0.5f, 0.5f, 0.0f, 0.0f);
}

/**
 * Creates the 3D ridge noise texture.
 */
void NoiseTextureKeeper::declareUsingRidgeNoise3DTexture() {
    mRidge3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mRidge3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mRidge3D->mParam.mParam0.set(10.0f, 1.0f, 1.0f, 1.0f);
}

/**
 * Creates the 3D caustics textures and the curl noise texture they are distorted with.
 */
void NoiseTextureKeeper::declareUsingCaustics3DTexture() {
    mCaustics3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 0);
    mCaustics3D->mParam.mScale.set(2.0f, 2.0f, 2.0f);
    mCaustics3D->mParam.mParam0.set(0.03f, 0.5f, 0.4f, 2.0f);
    mCaustics3D->mParam.mParam1.set(-0.125f, 0.0f, 0.0f, 0.0f);

    mCaustics3D1ch->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mCaustics3D1ch->copyParam(mCaustics3D);

    declareUsingCurlNoise3DTexture();
}

/**
 * Creates the 3D perlin noise texture.
 */
void NoiseTextureKeeper::declareUsingPerlinNoise3DTexture() {
    mPerlin3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mPerlin3D->mParam.mScale.set(2.0f, 2.0f, 1.0f);
    mPerlin3D->mParam.mParam0.set(0.4f, 1.0f, 0.6f, 0.0f);
}

/**
 * Creates the 3D perlin fbm noise texture.
 */
void NoiseTextureKeeper::declareUsingPerlinFbmNoise3DTexture() {
    mPerlinFbm3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mPerlinFbm3D->mParam.mScale.set(1.0f, 1.0f, 1.0f);
    mPerlinFbm3D->mParam.mParam0.set(0.0f, 0.0f, 0.0f, 0.0f);
}

/**
 * Creates the 3D gem noise texture.
 */
void NoiseTextureKeeper::declareUsingGemNoise3DTexture() {
    mGemNoise3D->create(32, 32, 32, agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 0);
    mGemNoise3D->mParam.mScale.set(1.0f, 1.0f, 1.0f);
    mGemNoise3D->mParam.mParam0.set(0.05f, 10.0f, 1.0f, 1.0f);
    mGemNoise3D->mParam.mParam1.set(6.15f, 0.0f, 0.0f, 0.0f);
}

}  // namespace al

/**
 * Writes a vector to a member of a uniform block.
 * @param pUniformBlock Uniform block to write.
 * @param index Index of the member.
 * @param rValue Value to write.
 */
static void setUboVector4f(al::UniformBlock* pUniformBlock, s32 index,
                           const sead::Vector4f& rValue) {
    pUniformBlock->setData(index, &rValue, 0, 1);
}

/**
 * Uploads the texture size and the noise parameters to the uniform block.
 */
inline void NoiseTexture::updateUbo() {
    if (!isValid()) {
        return;
    }

    const agl::TextureData* textureData = mTextureUnit->getTextureData();
    f32 width = textureData->getWidth(0);
    f32 height = textureData->getHeight(0);
    textureData = mTextureUnit->getTextureData();
    f32 depth = static_cast<u32>(textureData->getMipSlice(0));

    mUniformBlock->swap();
    setUboVector4f(mUniformBlock, 0,
                   sead::Vector4f(1.0f / width, 1.0f / height, 1.0f / depth, width / height));
    setUboVector4f(mUniformBlock, 1, sead::Vector4f(mParam.mTime, 0.0f, 0.0f, 1.0f));
    setUboVector4f(mUniformBlock, 2,
                   sead::Vector4f(mParam.mScale.x, mParam.mScale.y, mParam.mScale.z, 0.0f));
    setUboVector4f(mUniformBlock, 3,
                   sead::Vector4f(mParam.mParam0.x, mParam.mParam0.y, mParam.mParam0.z,
                                  mParam.mParam0.w));
    mUniformBlock->setData(4, &mParam.mParam1, 0, 1);
    mUniformBlock->setData(5, &mParam.mParam2, 0, 1);
    mUniformBlock->setData(6, &mParam.mParam3, 0, 1);
    mUniformBlock->setData(7, &mParam.mParam4, 0, 1);
    mUniformBlock->setData(5, &mParam.mParam2, 0, 1);

    u32 size = mUniformBlock->getBlockSize();
    u32 offset = mUniformBlock->getCurrentBlockOffset(0);
    agl::GPUMemVoidAddr buffer = mUniformBlock->getBuffer();
    agl::GPUMemVoidAddr(buffer, offset).flushCPUCache(size);
}

namespace al {

/**
 * Creates the simple noise texture.
 */
void NoiseTextureKeeper::declareUsingSimpleNoiseTexture() {
    mSimple->create(256, 256, 0, agl::TextureFormat::cTextureFormat_R8_uNorm, 1);
    mSimple->mParam.mScale.set(3.0f, 3.0f, 1.0f);
    mSimple->mParam.mParam0.set(3.0f, 3.0f, 8.0f, 0.0f);
}

/**
 * Creates the 3D cloud noise texture.
 */
void NoiseTextureKeeper::declareUsingCloudNoise3DTexture() {
    mCloud3D->create(128, 128, 128, agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 0);
    mCloud3D->mParam.mScale.set(4.0f, 4.0f, 4.0f);
    mCloud3D->mParam.mParam0.set(0.0f, 0.25f, 1.0f, 1.0f);
    mCloud3D->mParam.mParam1.set(0.25f, 1.0f, 0.0f, 0.0f);
}

/**
 * Gets the sampler of the 3D cloud noise texture.
 * @return Sampler of the 3D cloud noise texture.
 */
const agl::TextureSampler* NoiseTextureKeeper::getCloudVolume3DSampler() const {
    return mCloud3D->getSampler();
}

/**
 * Activates the gem noise cubemap texture. Does nothing in release builds.
 * @param pDrawContext Draw context.
 */
void NoiseTextureKeeper::activateGemNoiseCubemapTexture(agl::DrawContext* pDrawContext) const {}

/**
 * Activates a 2D noise texture. Does nothing in release builds.
 * @param pDrawContext Draw context.
 * @param index Index of the 2D noise texture.
 */
void NoiseTextureKeeper::activateTexture2D(agl::DrawContext* pDrawContext, s32 index) const {}

/**
 * Gets a 2D noise texture, falling back to the black texture when it was not created.
 * @param index 0: perlin fbm, 1: worley, 2: frost.
 * @return The noise texture.
 */
const NoiseTexture* NoiseTextureKeeper::getTexture2D(s32 index) const {
    switch (index) {
    case 0:
        if (mPerlinFbm2D->isValid()) {
            return mPerlinFbm2D;
        }

        break;
    case 1:
        if (mWorley2D->isValid()) {
            return mWorley2D;
        }

        break;
    case 2:
        if (mFrost2D->isValid()) {
            return mFrost2D;
        }

        break;
    }

    return mBlack2D;
}

/**
 * Gets a 3D noise texture, falling back to the black texture when it was not created.
 * @param index Index of the 3D noise texture, -1 for the black texture.
 * @return The noise texture.
 */
const NoiseTexture* NoiseTextureKeeper::getTexture3D(s32 index) const {
    switch (index) {
    case -1:
        if (mBlack3D->isValid()) {
            return mBlack3D;
        }

        break;
    case 0:
        if (mCaustics3D->isValid()) {
            return mCaustics3D;
        }

        break;
    case 1:
        if (mCaustics3D1ch->isValid()) {
            return mCaustics3D1ch;
        }

        break;
    case 2:
        if (mCloudLikeFbm3D->isValid()) {
            return mCloudLikeFbm3D;
        }

        break;
    case 3:
        if (mCurl3D->isValid()) {
            return mCurl3D;
        }

        break;
    case 4:
        if (mFrost3D->isValid()) {
            return mFrost3D;
        }

        break;
    case 5:
        if (mGemNoise3D->isValid()) {
            return mGemNoise3D;
        }

        break;
    case 6:
        if (mOceanFoam3D->isValid()) {
            return mOceanFoam3D;
        }

        break;
    case 7:
        if (mSnowCovered3D->isValid()) {
            return mSnowCovered3D;
        }

        break;
    case 8:
        if (mPerlin3D->isValid()) {
            return mPerlin3D;
        }

        break;
    case 9:
        if (mPerlinFbm3D->isValid()) {
            return mPerlinFbm3D;
        }

        break;
    case 10:
        if (mRidge3D->isValid()) {
            return mRidge3D;
        }

        break;
    case 11:
        if (mWorley3D->isValid()) {
            return mWorley3D;
        }

        break;
    case 12:
        if (mWorleyThin3D->isValid()) {
            return mWorleyThin3D;
        }

        break;
    case 13:
        if (mWorleyThinAnim3D->isValid()) {
            return mWorleyThinAnim3D;
        }

        break;
    }

    return mBlack3D;
}

/**
 * Gets the sampler of a 2D noise texture, falling back to the black texture.
 * @param index 0: perlin fbm, 1: worley, 2: frost.
 * @return Sampler of the noise texture.
 */
const agl::TextureSampler* NoiseTextureKeeper::getTexture2DSampler(s32 index) const {
    return getTexture2D(index)->getSampler();
}

/**
 * Gets the sampler of a 3D noise texture, falling back to the black texture.
 * @param index Index of the 3D noise texture.
 * @return Sampler of the noise texture.
 */
const agl::TextureSampler* NoiseTextureKeeper::getTexture3DSampler(s32 index) const {
    return getTexture3D(index)->getSampler();
}

/**
 * Activates a 3D noise texture. Does nothing in release builds.
 * @param pDrawContext Draw context.
 * @param index Index of the 3D noise texture.
 */
void NoiseTextureKeeper::activateTexture3D(agl::DrawContext* pDrawContext, s32 index) const {}

/**
 * Advances the noise animation time.
 * @param rInfo Update info.
 */
void NoiseTextureKeeper::update(const GraphicsUpdateInfo& rInfo) {
    mDeltaTime = rInfo.mDeltaTime;
    mTime += rInfo.mDeltaTime;
}

/**
 * Synchronizes the caustics parameters, advances the time of every noise texture and uploads
 * their parameters.
 * @param rInfo Calc gpu info.
 */
void NoiseTextureKeeper::calcGpu(const GraphicsCalcGpuInfo& rInfo) {
    if (mCaustics3D->mIsUpdated) {
        mCaustics3D1ch->copyParam(mCaustics3D);
    }

    if (mCaustics3D1ch->mIsUpdated) {
        mCaustics3D->copyParam(mCaustics3D1ch);
    }

    s32 size = mTextures.size();

    for (s32 i = 0; i < size; i++) {
        NoiseTexture* texture = mTextures.unsafeAt(i);

        if (!texture->isValid()) {
            continue;
        }

        texture->update(mDeltaTime);
    }
}

namespace {
/**
 * Applies the render state used to render the noise textures.
 * @param pDrawContext Draw context.
 */
void applyNoiseGraphicsContext(agl::DrawContext* pDrawContext) {
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setColorMask(true, true, true, true);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setCullingMode(0);
    graphicsContext.apply(pDrawContext);
}
}  // namespace

/**
 * Renders the noise textures whose parameters changed.
 * @param pInfo Render info.
 */
void NoiseTextureKeeper::drawSystem(const GraphicsRenderInfo* pInfo) const {
    agl::DrawContext* drawContext = pInfo->mDrawContext;
    agl::RenderBuffer renderBuffer;
    bool isAppliedContext = false;

    if (mBlack2D->isNeedDraw()) {
        mBlack2D->clear(drawContext);
    }

    if (mBlack3D->isNeedDraw()) {
        mBlack3D->clear(drawContext);
    }

    if (mCloudLikeFbm3D->isNeedDraw() && mCloudLikeFbmShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mCloudLikeFbmShader->searchVariation(1, macros, values);
        mCloudLikeFbm3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                       "NoiseTexture::CloudLikeFbmNoise", nullptr);
        isAppliedContext = true;
    }

    if (mCurl2D->isNeedDraw() && mCurlShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE", "IS_NORMALIZE"};
        const char* values[] = {"RENDER_TYPE", "IS_NORMALIZE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        index = ShaderSearchImpl::searchMacroIndex(macros, "IS_NORMALIZE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mCurlShader->searchVariation(2, macros, values);
        agl::UniformLocation depthLocation("uDepth");
        depthLocation.search(*program);
        depthLocation.setUniform(drawContext, 0.0f);
        mCurl2D->drawToTexture(drawContext, program, mFullScreenTriangle, "NoiseTexture::Curl",
                               nullptr);
        isAppliedContext = true;
    }

    if (mCurl3D->isNeedDraw() && mCurlShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE", "IS_NORMALIZE"};
        const char* values[] = {"RENDER_TYPE", "IS_NORMALIZE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        index = ShaderSearchImpl::searchMacroIndex(macros, "IS_NORMALIZE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mCurlShader->searchVariation(2, macros, values);
        mCurl3D->drawToTexture(drawContext, program, mFullScreenTriangle, "NoiseTexture::Curl3D",
                               nullptr);
        isAppliedContext = true;
    }

    if (mSimple->isNeedDraw() && mSimpleShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"IS_SEAMLESS"};
        const char* values[] = {"IS_SEAMLESS"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "IS_SEAMLESS");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mSimpleShader->searchVariation(1, macros, values);
        mSimple->drawToTexture(drawContext, program, mFullScreenTriangle, "NoiseTexture::Simple",
                               nullptr);
        isAppliedContext = true;
    }

    if (mCloud3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mCloud3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                "NoiseTexture::CloudNoise3D", nullptr);
        isAppliedContext = true;
    }

    if (mWorley2D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "1";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mWorley2D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                 "NoiseTexture::Worley2D", nullptr);
        isAppliedContext = true;
    }

    if (mWorley3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "1";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mWorley3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                 "NoiseTexture::Worley3D", nullptr);
        isAppliedContext = true;
    }

    if (mWorleyThin3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "4";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mWorleyThin3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                     "NoiseTexture::WorleyThin3D", nullptr);
        isAppliedContext = true;
    }

    if (mWorleyThinAnim3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "4";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mWorleyThinAnim3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                         "NoiseTexture::WorleyThinAnim3D", nullptr);
        isAppliedContext = true;
    }

    if (mFrost2D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "2";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mFrost2D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                "NoiseTexture::FrostNoise2D", nullptr);
        isAppliedContext = true;
    }

    if (mFrost3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "2";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mFrost3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                "NoiseTexture::FrostNoise3D", nullptr);
        isAppliedContext = true;
    }

    if (mPerlin2D->isNeedDraw() && mPerlinShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mPerlinShader->searchVariation(1, macros, values);
        mPerlin2D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                 "NoiseTexture::Perlin2D", nullptr);
        isAppliedContext = true;
    }

    if (mPerlin3D->isNeedDraw() && mPerlinShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mPerlinShader->searchVariation(1, macros, values);
        mPerlin3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                 "NoiseTexture::Perlin3D", nullptr);
        isAppliedContext = true;
    }

    if (mPerlinFbm2D->isNeedDraw() && mPerlinShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "1";
        }

        const agl::ShaderProgram* program = mPerlinShader->searchVariation(1, macros, values);
        mPerlinFbm2D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                    "NoiseTexture::PerlinFbm2D", nullptr);
        isAppliedContext = true;
    }

    if (mPerlinFbm3D->isNeedDraw() && mPerlinShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "1";
        }

        const agl::ShaderProgram* program = mPerlinShader->searchVariation(1, macros, values);
        mPerlinFbm3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                    "NoiseTexture::PerlinFbm3D", nullptr);
        isAppliedContext = true;
    }

    if (mRidge3D->isNeedDraw() && mPerlinShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "2";
        }

        const agl::ShaderProgram* program = mPerlinShader->searchVariation(1, macros, values);
        mRidge3D->drawToTexture(drawContext, program, mFullScreenTriangle, "NoiseTexture::Ridge3D",
                                nullptr);
        isAppliedContext = true;
    }

    if (mGemNoiseCubemap->isNeedDraw() && mGemShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "1";
        }

        const agl::ShaderProgram* program = mGemShader->searchVariation(1, macros, values);
        mGemNoiseCubemap->drawToTexture(drawContext, program, mFullScreenTriangle,
                                        "NoiseTexture::GemNoiseCubemap", mCubeMapDrawInfo);
        isAppliedContext = true;
    }

    if (mGemNoise3D->isNeedDraw() && mGemShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mGemShader->searchVariation(1, macros, values);
        mGemNoise3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                   "NoiseTexture::GemNoise3D", mCubeMapDrawInfo);
        isAppliedContext = true;
    }

    if (mCaustics3D->isNeedDraw() && mCausticsShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "0";
        }

        const agl::ShaderProgram* program = mCausticsShader->searchVariation(1, macros, values);
        agl::SamplerLocation noiseLocation("cNoise3D");
        noiseLocation.search(*program);

        if (noiseLocation.isValid()) {
            mCurl3D->getSampler()->activate(drawContext, noiseLocation, -1, false);
        }

        mCaustics3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                   "NoiseTexture::Caustics3D", nullptr);
        isAppliedContext = true;
    }

    if (mCaustics3D1ch->isNeedDraw() && mCausticsShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "1";
        }

        const agl::ShaderProgram* program = mCausticsShader->searchVariation(1, macros, values);
        agl::SamplerLocation noiseLocation("cNoise3D");
        noiseLocation.search(*program);

        if (noiseLocation.isValid()) {
            mCurl3D->getSampler()->activate(drawContext, noiseLocation, -1, false);
        }

        mCaustics3D1ch->drawToTexture(drawContext, program, mFullScreenTriangle,
                                      "NoiseTexture::Caustics3D1ch", nullptr);
        isAppliedContext = true;
    }

    if (mOceanFoam3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "3";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mOceanFoam3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                    "NoiseTexture::OceanFoam3D", nullptr);
        isAppliedContext = true;
    }

    if (mSnowCovered3D->isNeedDraw() && mWorleyShader != nullptr) {
        if (!isAppliedContext) {
            applyNoiseGraphicsContext(drawContext);
        }

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"RENDER_TYPE"};
        s32 index = ShaderSearchImpl::searchMacroIndex(macros, "RENDER_TYPE");

        if (index != -1) {
            values[index] = "3";
        }

        const agl::ShaderProgram* program = mWorleyShader->searchVariation(1, macros, values);
        mSnowCovered3D->drawToTexture(drawContext, program, mFullScreenTriangle,
                                      "NoiseTexture::SnowCovered3D", nullptr);
        isAppliedContext = true;
    }
}

}  // namespace al
