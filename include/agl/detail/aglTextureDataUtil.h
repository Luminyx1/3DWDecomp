#pragma once

#include <basis/seadTypes.h>
#include <nn/gfx/gfx_Enum.h>
#include <nvn/nvn.h>
#include "common/aglTextureEnum.h"

namespace agl::detail {

class TextureDataUtil {
public:
    static TextureFormat convFormatDriverToAGL(NVNformat format);
    static NVNformat convFormatAGLToDriver(TextureFormat format);
    static TextureCompSel convCompSelDriverToAGL(NVNtextureSwizzle swizzle);
    static TextureFormat convFormatNNGfxToAGL(nn::gfx::ImageFormat format);
};

}  // namespace agl::detail
