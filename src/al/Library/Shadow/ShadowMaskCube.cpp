#include "Library/Shadow/ShadowMaskCube.hpp"

#include <agl/common/aglTextureSampler.h>

#include "Library/Shadow/ShadowMaskDrawer.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
void makeTextureDataFromArchive(agl::TextureData* pTextureData, const char* pArchiveName,
                                const char* pFileName, const char* pTextureName);

/**
 * Constructs a cube shadow mask.
 * @param pName Name of the shadow mask.
 */
ShadowMaskCube::ShadowMaskCube(const char* pName)
    : ShadowMaskBase(pName), mScale(103.0f, 500.0f, 103.0f), mExp(100.0f, 3.0f, 100.0f),
      mDistYBase(0.5f), _108(false), _109(false), mTextureSampler(nullptr),
      mTextureBaseName(nullptr), _248(sead::Vector2f::zero), _250(sead::Vector2f::zero),
      _258(sead::Vector2f::zero), _260(sead::Vector2f::zero), _268(sead::Vector2f::zero),
      mTextureFixedScale(5.0f) {}

/**
 * Loads the projection texture of the cube.
 * @param pTextureName Base name of the texture.
 */
void ShadowMaskCube::tryInitTexture(const char* pTextureName) {
    if (!pTextureName) {
        mTextureBaseName = "None";
        return;
    }
    if (isEqualString(pTextureName, "None")) {
        return;
    }
    mTextureBaseName = pTextureName;
    StringTmp<128> archiveName("ObjectData/ProjTex%s", pTextureName);
    StringTmp<128> fileName("ProjTex%s", mTextureBaseName);
    StringTmp<128> textureName("ProjTex%s", mTextureBaseName);
    makeTextureDataFromArchive(&mTextureData, archiveName.cstr(), fileName.cstr(),
                               textureName.cstr());
    mTextureSampler = new agl::TextureSampler(mTextureData);
    mTextureSampler->setWrapDirect(1, 1, 1);
}

/**
 * Destroys the cube shadow mask and its texture sampler.
 */
ShadowMaskCube::~ShadowMaskCube() {
    if (mTextureSampler) {
        delete mTextureSampler;
        mTextureSampler = nullptr;
    }
}

/**
 * Declares the shadow mask to the shadow mask keeper.
 * @param category Draw category.
 */
void ShadowMaskCube::declare(ShadowMaskDrawCategory category) {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->declare(ShadowMaskType::Cube, category);
}

}  // namespace al
