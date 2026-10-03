#include <nn/gfx/gfx_BufferInfo.h>

namespace nn::gfx {

/**
 * Resets the buffer info to its defaults (zero size and no GPU access flags).
 */
void BufferInfo::SetDefault() {
    SetSize(0);
    SetGpuAccessFlags(0);
}

/**
 * Resets the buffer texture view info to its defaults: an undefined format and an empty view with
 * no buffer.
 */
void BufferTextureViewInfo::SetDefault() {
    SetImageFormat(ImageFormat_Undefined);
    SetOffset(0);
    SetSize(0);
    SetBufferPtr(nullptr);
}

}  // namespace nn::gfx
