#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace nn::ui2d {
struct ResBounding;

class Bounding : public Pane {
public:
    Bounding() = default;
    Bounding(const ResBounding*, const ResBounding*, const BuildArgSet&);
    Bounding(const Bounding&);
    ~Bounding() override;
    NN_RUNTIME_TYPEINFO(Pane);
    void DrawSelf(DrawInfo&, nn::gfx::CommandBuffer&) override;
    bool CompareCopiedInstanceTest(const Bounding&) const;
};
}  // namespace nn::ui2d
