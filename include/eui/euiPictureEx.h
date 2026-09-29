#pragma once

#include <nn/ui2d/ui2d_Picture.h>

namespace eui {
class PictureEx : public nn::ui2d::Picture {
public:
    explicit PictureEx(u8 textureCount);
    explicit PictureEx(const nn::ui2d::TextureInfo& rTexture);
    PictureEx(const nn::ui2d::ResPicture* pResource, const nn::ui2d::ResPicture* pOverride,
              const nn::ui2d::BuildArgSet& rArgs);
    PictureEx(const PictureEx& rOther);
    /** @brief Destroys the extended picture pane. */
    ~PictureEx() override = default;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Picture);

private:
    void setupForSingleTexture_();
    void initializeVertexColor_();
};
}  // namespace eui
