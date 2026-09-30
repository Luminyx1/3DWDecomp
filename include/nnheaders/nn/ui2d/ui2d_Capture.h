#pragma once
#include <nn/ui2d/ui2d_Pane.h>
namespace nn::ui2d {
struct ResCapture;
class Capture : public Pane {
public:
    Capture(const ResCapture* resource, const ResCapture* overrideResource, const BuildArgSet& args);
    Capture(const Capture& source);
    ~Capture() override;
    NN_RUNTIME_TYPEINFO(Pane);
    void Calculate(DrawInfo&, CalculateContext&, bool) override;
    void Draw(DrawInfo& info, nn::gfx::CommandBuffer& commandBuffer) override;
    bool CompareCopiedInstanceTest(const Capture& source) const;
};
}
