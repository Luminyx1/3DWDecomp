#include <eui/euiUtility.h>

#include <driver/aglNVNMgr.h>
#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_Texture.h>
#include <nn/gfx/gfx_Sampler.h>

namespace eui {

/**
 * Reverses a cardinal direction.
 * @param direction Direction to reverse; an invalid value is treated as up.
 * @return The opposite direction.
 */
Direction GetOppositeDirection(Direction direction)
{
    switch (direction.value()) {
    case Direction::cDown: return Direction::cUp;
    case Direction::cLeft: return Direction::cRight;
    case Direction::cRight: return Direction::cLeft;
    default: return Direction::cDown;
    }
}

/**
 * Converts a cardinal direction to an angle measured counterclockwise from right.
 * @param direction Direction to convert; invalid values return zero.
 * @return The angle in radians.
 */
f32 GetRadAngleOfDirection(Direction direction)
{
    switch (direction.value()) {
    case Direction::cUp: return 1.5707963705062866f;
    case Direction::cDown: return 4.71238899230957f;
    case Direction::cLeft: return 3.1415927410125732f;
    default: return 0.0f;
    }
}

/**
 * Registers a texture descriptor with the graphics driver.
 * @param pSlot Receives the descriptor ID.
 * @param rView Texture and view to register.
 * @param pUserData Unused callback context.
 * @return Always true.
 */
bool RegisterSlotForTexture(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::TextureView& rView,
                            void* pUserData)
{
    auto data = rView.ToData();
    pSlot->ToData()->value = static_cast<u32>(agl::driver::NVNMgr::instance()->registerTexture(
        static_cast<const NVNtexture*>(data->pNvnTexture.ptr),
        static_cast<const NVNtextureView*>(data->pNvnTextureView.ptr), "ui2d"));
    return true;
}

/**
 * Registers a sampler descriptor with the graphics driver.
 * @param pSlot Receives the descriptor ID.
 * @param rSampler Sampler to register.
 * @param pUserData Unused callback context.
 * @return Always true.
 */
bool RegisterSlotForSampler(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::Sampler& rSampler,
                            void* pUserData)
{
    pSlot->ToData()->value = agl::driver::NVNMgr::instance()->registerSampler(
        static_cast<const NVNsampler*>(rSampler.ToData()->pNvnSampler.ptr), "ui2d");
    return true;
}

/**
 * Releases a registered texture descriptor.
 * @param pSlot Slot containing the descriptor ID to release.
 * @param rView Unused texture view.
 * @param pUserData Unused callback context.
 */
void UnregisterSlotForTexture(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::TextureView& rView,
                              void* pUserData)
{
    agl::driver::NVNMgr::instance()->releaseTexture(pSlot->ToData()->value);
}

/**
 * Releases a registered sampler descriptor.
 * @param pSlot Slot containing the descriptor ID to release.
 * @param rSampler Unused sampler.
 * @param pUserData Unused callback context.
 */
void UnregisterSlotForSampler(nn::gfx::DescriptorSlot* pSlot, const nn::gfx::Sampler& rSampler,
                              void* pUserData)
{
    agl::driver::NVNMgr::instance()->releaseSampler(pSlot->ToData()->value);
}

}  // namespace eui

