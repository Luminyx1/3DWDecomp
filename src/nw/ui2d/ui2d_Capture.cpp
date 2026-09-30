#include <nn/ui2d/ui2d_Capture.h>
namespace nn::ui2d {
// resource supplies pane settings and args supplies build context; overrideResource is unused.
Capture::Capture(const ResCapture* resource, const ResCapture* overrideResource, const BuildArgSet& args)
    : Pane(nullptr, nullptr, reinterpret_cast<const ResPane*>(resource), args) {}
// source supplies the pane state to copy.
Capture::Capture(const Capture& source) : Pane(source) {}
Capture::~Capture() = default;
// info and commandBuffer are unused: captures are rendered in the capture-texture pass.
void Capture::Draw(DrawInfo& info, nn::gfx::CommandBuffer& commandBuffer) {}
// source is unused by this capture-specific comparison hook.
bool Capture::CompareCopiedInstanceTest(const Capture& source) const { return true; }
}
