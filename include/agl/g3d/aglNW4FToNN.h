#pragma once

#include <basis/seadTypes.h>

namespace nn::g3d {
class TextureRef;
class MaterialAnimObj;
class MaterialObj;
class ResFile;
class ResMaterial;
class ResMaterialAnim;
class ShapeObj;
}  // namespace nn::g3d

namespace nn::gfx {
class ResTexture;
class ResTextureFile;
}  // namespace nn::gfx

namespace agl::g3d {

class ResFile {
public:
    static void Setup(nn::g3d::ResFile* pResFile);
    static void Cleanup(nn::g3d::ResFile* pResFile);
    static nn::gfx::ResTextureFile* getResTextureFile(nn::g3d::ResFile* pResFile);
    static const nn::gfx::ResTextureFile* getResTextureFile(const nn::g3d::ResFile* pResFile);
    static const char* GetTextureName(const nn::g3d::ResFile* pResFile, s32 index);
    static s32 GetTextureCount(const nn::g3d::ResFile* pResFile);
    static s32 GetTextureIndex(const nn::g3d::ResFile* pResFile, const char* pName);
    static nn::gfx::ResTexture* GetTexture(nn::g3d::ResFile* pResFile, const char* pName);
    static const nn::gfx::ResTexture* GetTexture(const nn::g3d::ResFile* pResFile,
                                                 const char* pName);
    static nn::gfx::ResTexture* GetTexture(nn::g3d::ResFile* pResFile, s32 index);
    static const nn::gfx::ResTexture* GetTexture(const nn::g3d::ResFile* pResFile, s32 index);
    static bool BindTexture(nn::g3d::ResFile* pResFile, const nn::g3d::ResFile* pTextureFile);
    static bool BindSharedTexture(nn::g3d::ResFile* pResFile,
                                  const nn::g3d::ResFile* pTextureFile);

private:
    static nn::g3d::TextureRef TextureBindCallback(const char* pName, void* pUserData);
    static nn::g3d::TextureRef SharedTextureBindCallback(const char* pName, void* pUserData);
};

class ResTexture {
public:
    static void Initialize(nn::gfx::ResTexture* pResTexture);
    static void Finalize(nn::gfx::ResTexture* pResTexture);
    static bool IsInitialized(const nn::gfx::ResTexture* pResTexture);
};

class ResMaterial {
public:
    static nn::gfx::ResTexture* GetTexture(nn::g3d::ResMaterial* pMaterial, s32 index);
    static const nn::gfx::ResTexture* GetTexture(const nn::g3d::ResMaterial* pMaterial,
                                                 s32 index);
    static void ForceBindTexture(nn::g3d::ResMaterial* pMaterial, s32 index,
                                 const nn::gfx::ResTexture* pTexture);
    static const char* GetTextureName(const nn::g3d::ResMaterial* pMaterial, s32 index);
    static void ReleaseTexture(nn::g3d::ResMaterial* pMaterial, s32 index);
};

class ResMaterialAnim {
public:
    static nn::gfx::ResTexture* GetTexture(nn::g3d::ResMaterialAnim* pAnim, s32 index);
    static const nn::gfx::ResTexture* GetTexture(const nn::g3d::ResMaterialAnim* pAnim,
                                                 s32 index);
    static const char* GetTextureName(const nn::g3d::ResMaterialAnim* pAnim, s32 index);
    static s32 GetTextureCount(const nn::g3d::ResMaterialAnim* pAnim);
    static void ForceBindTexture(nn::g3d::ResMaterialAnim* pAnim, s32 index,
                                 const nn::gfx::ResTexture* pTexture);
    static void ReleaseTexture(nn::g3d::ResMaterialAnim* pAnim, s32 index);
};

class MaterialObj {
public:
    static nn::gfx::ResTexture* GetResTexture(nn::g3d::MaterialObj* pMaterial, s32 index);
    static const nn::gfx::ResTexture* GetResTexture(const nn::g3d::MaterialObj* pMaterial,
                                                    s32 index);
    static const char* GetResTextureName(const nn::g3d::MaterialObj* pMaterial, s32 index);
    static void SetResTexture(nn::g3d::MaterialObj* pMaterial, s32 index,
                              nn::gfx::ResTexture* pTexture);
    static void ClearTexture(nn::g3d::MaterialObj* pMaterial);
    static void* Map(nn::g3d::MaterialObj* pMaterial, s32 bufferIndex);
    static void FlushAndUnmap(nn::g3d::MaterialObj* pMaterial, s32 bufferIndex);
};

class MaterialAnimObj {
public:
    static void SetResTexture(nn::g3d::MaterialAnimObj* pAnim, s32 index,
                              nn::gfx::ResTexture* pTexture);
    static nn::gfx::ResTexture* GetResTexture(nn::g3d::MaterialAnimObj* pAnim, s32 index);
    static const nn::gfx::ResTexture* GetResTexture(const nn::g3d::MaterialAnimObj* pAnim,
                                                    s32 index);
};

class ShapeObj {
public:
    static void* Map(nn::g3d::ShapeObj* pShape, s32 bufferIndex, s32 viewIndex);
    static void FlushAndUnmap(nn::g3d::ShapeObj* pShape, s32 bufferIndex, s32 viewIndex);
};

}  // namespace agl::g3d
