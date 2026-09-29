/**
 * @file Material.h
 * @brief UI Material implementation.
 */

#pragma once

#include <nn/types.h>
#include <nn/ui2d/ui2d_Types.h>

namespace nn {
namespace ui2d {
class AnimTransform;
class BuildResultInformation;
struct UserShaderInformation;

class TexMap {
public:
    void SetWrapMode(TexWrap, TexWrap);
    void SetFilter(TexFilter, TexFilter);
};

class Material {
public:
    Material();

    void Initialize();
    void ReserveMem(s32, s32, s32, s32, bool, s32, bool, s32, bool, bool);
    void SetupUserShaderConstantBufferInformation(nn::ui2d::UserShaderInformation const&);

    virtual ~Material();
    virtual void BindAnimation(nn::ui2d::AnimTransform*);
    virtual void UnbindAnimation(nn::ui2d::AnimTransform*);

    void SetTextureNum(u8 count) { mTextureCount = (mTextureCount & 0xf) | (count << 4); }
    TexMap* GetFirstTexMap() { return m_pTexMaps; }

    unsigned char _08[0x10];
    TexMap* m_pTexMaps;
    unsigned char _20[0x28];
    u8 mTextureCount;
};
}  // namespace ui2d
}  // namespace nn
