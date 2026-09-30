/**
 * @file Material.h
 * @brief UI Material implementation.
 */

#pragma once

#include <nn/types.h>
#include <nn/ui2d/ui2d_Types.h>
#include <nn/ui2d/ui2d_TexMap.h>

namespace nn {
namespace ui2d {
class AnimTransform;
class BuildResultInformation;
struct UserShaderInformation;
class TextureInfo;

class Material {
public:
    Material();

    void Initialize();
    void ReserveMem(s32, s32, s32, s32, bool, s32, bool, s32, bool, bool);
    void SetupUserShaderConstantBufferInformation(nn::ui2d::UserShaderInformation const&);
    size_t GetVertexShaderConstantBufferSize() const;
    size_t GetPixelShaderConstantBufferSize() const;
    size_t GetGeometryShaderConstantBufferSize() const;
    size_t GetPixelShaderDetailedCombinerConstantBufferSize() const;
    size_t GetPixelShaderCombinerUserShaderConstantBufferSize() const;
    bool IsUseFramebufferTexture() const;

    virtual ~Material();
    virtual void BindAnimation(nn::ui2d::AnimTransform*);
    virtual void UnbindAnimation(nn::ui2d::AnimTransform*);

    void SetTextureNum(u8 count) { mTextureCount = (mTextureCount & 0xf) | (count << 4); }
    TexMap* GetFirstTexMap() { return m_pTexMaps; }
    const u8* GetWhiteColor() const { return &_08[4]; }
    // index selects the material slot; pInfo supplies its texture description.
    void SetTextureInfo(int index, const TextureInfo* pInfo) {
        m_pTexMaps[index].m_pTextureInfo = pInfo;
    }

    union { unsigned char _08[0xc]; struct { u32 mBlackColor; u32 mWhiteColor; u32 mResourceCapacity; }; };
    u32 mResourceCounts;
    TexMap* m_pTexMaps;
    union { unsigned char _20[8]; void* mShaderInfo; };
    const char* mName;
    struct UserShaderConstantBufferInformation {
        u32 vertexSize, pixelSize, geometrySize, flags;
    };
    union { unsigned char _30[0x18]; struct { void* mUserShader; UserShaderConstantBufferInformation* mUserShaderConstantBufferInformation; void* mDetailedCombiner; }; };
    union { u32 mFlags; struct { u8 mTextureCount; u8 mOwnershipFlags; u16 mShaderVariation; }; };
};
}  // namespace ui2d
}  // namespace nn
