#pragma once
#include <nn/ui2d/ui2d_Pane.h>
#include <nn/gfx/gfx_StateInfo.h>
namespace nn::ui2d {
struct ResScissor;
class Scissor : public Pane {
public:
    Scissor();
    Scissor(const ResScissor* resource, const BuildArgSet& args);
    ~Scissor() override;
    NN_RUNTIME_TYPEINFO(Pane);
    void SetScissorStateInfoValue(nn::gfx::ScissorStateInfo* scissor, float x, float y, float width, float height);
    void Draw(DrawInfo& info, nn::gfx::CommandBuffer& commands) override;
};
}
