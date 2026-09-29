#pragma once

#include <prim/seadSafeString.h>

namespace nn::gfx {
class ResTexture;
}  // namespace nn::gfx

namespace agl {
class TextureData;
}  // namespace agl

namespace agl::g3d {

class TextureDataInitializerG3D {
public:
    static void initialize(TextureData* pTextureData, void* pFile, s32 index);
    static void initialize(TextureData* pTextureData, nn::gfx::ResTexture& rResTexture);
    static void initialize(TextureData* pTextureData, void* pFile, const sead::SafeString& rName);
};

}  // namespace agl::g3d
